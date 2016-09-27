#ifdef WIN32
#include "StdAfx.h"
#include "Win32DataStream.h"
#include "Constants/AllocTypes.h"
#include "Platforms/Win32Helpers.h"

using namespace LeEK;

void Win32DataStream::reset()
{
	memset(&ovrLp, 0, sizeof(OVERLAPPED));
	fileHnd = INVALID_HANDLE_VALUE;
	fileSize = 0;
	isOverlapped = false;
	//asyncThreadHnd = INVALID_HANDLE_VALUE;
	asyncThreadID = 0;
	//abortStreamEvt = INVALID_HANDLE_VALUE;
}

U32 Win32DataStream::getFilePos()
{
	LARGE_INTEGER filePos = {0};
	LARGE_INTEGER zero = {0};
	SetFilePointerEx(fileHnd, zero, &filePos, FILE_CURRENT);

	//Danger - truncation!
	return (U32)filePos.QuadPart;
}

Win32DataStream::Win32DataStream(const char* path, StreamFlag flags)
{
	reset();
	Open(path, flags);
}

Win32DataStream::~Win32DataStream()
{
	Close();
	//Delete the converted path string we made.
	LFree(path);
}


//Converts engine stream flags to Win32 file access flags.
DWORD convertAccessFlags(StreamFlag flags)
{
	DWORD result = 0;
	if(flags & StreamFlags::Read)
	{
		result |= GENERIC_READ;
	}
	if(flags & StreamFlags::Write)
	{
		result |= GENERIC_WRITE;
	}
	return result;
}

//Converts engine stream flags to Win32 IO flags.
DWORD convertIOFlags(StreamFlag flags)
{
	/*
	DWORD result = 0;
	//If we're going to open an async stream, we can't use
	//FILE_ATTRIBUTE_NORMAL at all.

	if(flags & StreamFlags::Async)
	{
		result = FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED;
		return result;
	}
	else
	{
		result |= FILE_ATTRIBUTE_NORMAL;
	}
	*/
	return FILE_ATTRIBUTE_NORMAL;
}

DWORD WINAPI DataStreamProc( LPVOID pContext )
{
	//Sanity check - quit if we didn't get a context.
	if(!pContext)
	{
		return -1;
	}

	StreamContext* sc = (StreamContext*)pContext;
	IAsyncCallback* callbacks = sc->Callbacks;
	DefaultAsyncCallback defaultCallbacks = DefaultAsyncCallback();
	if(!callbacks)
	{
		callbacks = &defaultCallbacks;
		L_ASSERT(callbacks && "Couldn't set default callbacks for async stream!");
	}
	//Try to open the file.
	HANDLE fileHnd = CreateFile(sc->Path,
								convertAccessFlags(sc->Flags),
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,//FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
								NULL);

	//If we failed to open the file, notify callback and quit.
	if(fileHnd == INVALID_HANDLE_VALUE)
	{
		callbacks->OnError();
		//Shutdown the file handle gracefully.
		return -1;
	}

	//Try to allocate buffers, too.
	//This is RAII, so it'll shutdown properly on procedure exit.
	AsyncBuffer buffers = AsyncBuffer(sc->ChunkSize, sc->NumChunks);
	/*char** buffers = (char**)LMalloc(sizeof(char*)*sc->NumChunks, AllocType::FILESYS_ALLOC, "AsyncBufAlloc");
	for(int i = 0; i < sc->NumChunks; ++i)
	{
		buffers[i] = (char*)LMalloc(sc->ChunkSize, AllocType::FILESYS_ALLOC, "AsyncBufAlloc");
	}*/

	//Otherwise, we're open. Notify callback...
	callbacks->OnStreamOpen();

	DWORD bytesRead = 0;
	//Now check our request type and try to fulfill.
	if(sc->AsyncType == AsyncRequest::AsyncRead)
	{
		//Try to read a chunk into the async buffer.
		bool keepReading = true;
		while(keepReading)
		{
			keepReading = ReadFile(fileHnd, buffers.GetCurrBuffer(), sc->ChunkSize, (LPDWORD)&bytesRead, NULL);
			//Swap buffers as necessary.
			//If there was an error, notify callback and quit.
			if(!keepReading)
			{
				callbacks->OnError();
				CloseHandle(fileHnd);
				return -1;
			}
			//Otherwise, if we reached EOF, exit the read loop.
			if(bytesRead == 0)
			{
				keepReading = false;
			}
			//*other otherwise*, tell callback about the read chunk.
			else
			{
				callbacks->OnChunkRead(bytesRead);
			}
		}
		callbacks->OnReadEnd();
	}
	else if(sc->AsyncType == AsyncRequest::AsyncWrite)
	{
		FileSz totalWritten = 0;
		DWORD numWritten = 0;
		//Try to write a chunk to disk.
		char* bufHead = sc->Buffer;
		while(totalWritten < sc->BufferSz)
		{
			FileSz numToWrite = Math::Min(sc->ChunkSize, (FileSz)(sc->BufferSz - totalWritten));
			//If the write fails, notify callback and quit.
			if(!WriteFile(fileHnd, bufHead, numToWrite, (LPDWORD)&numWritten, NULL))
			{
				callbacks->OnError();
				CloseHandle(fileHnd);
				return -1;
			}
			//Move the head by the number of bytes written.
			totalWritten += numWritten;
			bufHead += numWritten;
			//And notify callbacks that data was written.
			callbacks->OnChunkWritten(totalWritten);
		}
		callbacks->OnWriteEnd();
	}

	//Request is done; notify callback of success and quit.
	callbacks->OnStreamClose();
	CloseHandle(fileHnd);
	return 0;
}

bool Win32DataStream::DoAsync(AsyncRequest asyncType, char* buffer, FileSz bufSz, IAsyncCallback* asyncCallback)
{
	//Set up the thread context.
	StreamContext sc = {path,
						flags,
						asyncType,
						asyncCallback,
						ASYNC_CHUNK_SZ,
						ASYNC_CHUNK_COUNT,
						buffer,
						bufSz};

	/*
	//Setup the failure event to close threads on failure.
	abortStreamEvt = CreateEvent(	NULL,	//default security
									TRUE,	//do not automatically reset the event
									FALSE,	//event not set at first
									NULL	//don't give this event a name
									);
									*/

	//Now set up the IO thread.
	HANDLE asyncThreadHnd = CreateThread(	NULL,			//default attributes
											0,				//default stack size
											DataStreamProc, //the thread's procedure
											&sc,			//the thread's context
											0,				//default creation flags
											&asyncThreadID	//send thread ID to this var
											);
	
	//If thread setup failed, notify that things went bad.
	if(asyncThreadHnd == INVALID_HANDLE_VALUE)
	{
		if(asyncCallback)
		{
			asyncCallback->OnError();
		}
		return false;
	}

	//Immediately close the thread handle
	//so OS knows it can throw away the thread
	//when it stops executing.
	CloseHandle(asyncThreadHnd);

	//And report that we started the async operation!
	return true;
}

bool Win32DataStream::Open(const char* pPath, StreamFlag pFlags)
{
	//path = pPath;
	flags = pFlags;
	//Convert the path to whatever Win32 expects.
	path = Win32Helpers::MultiToWide(pPath, strlen(pPath));
	//Use the stream flags to figure out if we're async.
	//isOverlapped = (flags & StreamFlags::Async) != 0;
	//If we're synchronous, open in the main thread.
	//Open the file!
	fileHnd = CreateFile(	path,
							convertAccessFlags(flags),
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							convertIOFlags(flags),
							NULL);
	updateFileSize();

	//If we failed to open the file, do stuff.
	if(fileHnd == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}
/*
bool Win32DataStream::OpenReadOnly(const char* path)
{
	wchar_t* convertedPath = Win32Helpers::MultiToWide(path, strlen(path));
	//open a read only file
	fileHnd = CreateFile(	convertedPath,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	updateFileSize();

	if(fileHnd == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	isOverlapped = false;
	return true;
}

bool Win32DataStream::Open(const char* path)
{
	//open a read-write file
	wchar_t* convertedPath = Win32Helpers::MultiToWide(path, strlen(path));
	//open a read only file
	fileHnd = CreateFile(	convertedPath,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	updateFileSize();

	if(fileHnd == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	isOverlapped = false;
	return true;
}

bool Win32DataStream::OpenAsync(const char* path)
{
	wchar_t* convertedPath = Win32Helpers::MultiToWide(path, strlen(path));
	//open a read only file
	fileHnd = CreateFile(	convertedPath,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
							NULL);
	updateFileSize();

	if(fileHnd == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	isOverlapped = true;
	return true;
}

bool Win32DataStream::OpenText(const char* path)
{
	return Open(path);
}
*/
bool Win32DataStream::Close()
{
	CloseHandle(fileHnd);
	reset();

	//if file's invalid, there wasn't anything to close
	return true;
}

FileSz Win32DataStream::Write(const char* buffer, FileSz size)
{
	U32 numWritten = 0;
	//If you use an OVERLAPPED struct in a non-overlapped
	//file, the system will use the struct's offset into the file
	//as the cursor position AND update it after the write's done.
	//Thus we must always use the overlapped struct if we don't
	//want to do some kind of branching on I/O calls.
	WriteFile(fileHnd, buffer, size, (LPDWORD)&numWritten, &ovrLp);
	//update file size?
	updateFileSize();
	return numWritten;
}

FileSz Win32DataStream::Write(const char* buffer)
{
	return Write(buffer, strlen(buffer));
}

FileSz Win32DataStream::WriteLine(const char* line)
{
	FileSz res = Write(line);
	return res + Write("\r\n");
}

FileSz Win32DataStream::Read(char* buffer, FileSz size)
{
	U32 numRead = 0;
	ReadFile(fileHnd, buffer, size, (LPDWORD)&numRead, &ovrLp); 
	return numRead;
}

FileSz Win32DataStream::WritePos()
{
	return getFilePos();
}

FileSz Win32DataStream::ReadPos()
{
	return getFilePos();
}

char* Win32DataStream::ReadAll()
{
	//make buffer
	FileSz fileSize = FileSize();
	char* buffer = CustomArrayNew<char>(fileSize + 1, FILESYS_ALLOC, "FileBufAlloc");//new char[fileSize + 1];
	//read the data in
	Read(buffer, fileSize);
	//null-terminate the buffer in case it needs to be displayed
	buffer[fileSize] = 0;
	//return the buffer
	return buffer;
	//return Read(buffer, fileSize);
}

FileSz Win32DataStream::Seek(FileSz position)
{
	//clamp position under file size
	FileSz finalPos = (position > fileSize) ? fileSize : position;
	LARGE_INTEGER convertedPos = {0};
	convertedPos.QuadPart = finalPos;
	SetFilePointerEx(fileHnd, convertedPos, NULL, FILE_CURRENT);
	
	//we don't have separate read/write heads, just return normal
	return 0;
}

inline void Win32DataStream::updateFileSize()
{
	LARGE_INTEGER lrgFileSize = {0};
	if(!GetFileSizeEx(fileHnd, &lrgFileSize))
	{
		fileSize = 0;
		return;
	}
	//Very bad! Does truncation...
	fileSize = (FileSz)lrgFileSize.QuadPart;
}
#endif	//WIN32