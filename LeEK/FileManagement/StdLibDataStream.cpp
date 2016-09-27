#include "StdAfx.h"
#include "StdLibDataStream.h"
#include "Constants/AllocTypes.h"
using namespace LeEK;

std::ios_base::openmode convertFlags(StreamFlag flags)
{
	using namespace std;
	ios_base::openmode result = 0;
	if(flags & StreamFlags::Read)
	{
		result |= fstream::in;
	}
	if(flags & StreamFlags::Write)
	{
		result |= fstream::out;
	}
	//If we *don't* have a text flag, open in binary mode.
	if(~(flags & StreamFlags::Text))
	{
		result |= fstream::binary;
	}
	//In general, we want to append.
	result |= fstream::app;
	return result;
}

bool StdLibDataStream::Open(const char* path, StreamFlag flags, IAsyncCallback* asyncCallback)
{
	using namespace std;
	//Convert flags to fstream flags and open.
	file.open(path, convertFlags(flags));
	//to find file size, we have to go to the end of the file,
	//but also return the stream's position to its start, so everything's as users might expect
	updateFileSize();

	if(file.fail())
	{
		return false;
	}
	return true;
}

/*
bool StdLibDataStream::OpenReadOnly(const char* path)
{
	//open a read only file
	using namespace std;
	file.open(path, fstream::in | fstream::binary);
	updateFileSize();

	if(file.fail())
	{
		return false;
	}
	return true;
}

bool StdLibDataStream::Open(const char* path)
{
	//open a read-write file ("update", in this case)
	using namespace std;
	file.open(path, fstream::in | fstream::out | fstream::binary | fstream::app);
	//to find file size, we have to go to the end of the file,
	//but also return the stream's position to its start, so everything's as users might expect
	updateFileSize();

	if(file.fail())
	{
		return false;
	}
	return true;
}

bool StdLibDataStream::OpenText(const char* path)
{
	//open a read-write file ("update", in this case)
	using namespace std;
	file.open(path, fstream::in | fstream::out | fstream::app);
	//to find file size, we have to go to the end of the file,
	//but also return the stream's position to its start, so everything's as users might expect
	updateFileSize();

	if(file.fail())
	{
		return false;
	}
	return true;
}
*/

bool StdLibDataStream::Close()
{
	file.close();
	//if file's invalid, there wasn't anything to close
	return true;
}

FileSz StdLibDataStream::Write(const char* buffer, FileSz size)
{
	file.write(buffer, size);
	//update file size?
	updateFileSize();
	return size;
}

FileSz StdLibDataStream::Write(const char* buffer)
{
	return Write(buffer, strlen(buffer));
}

FileSz StdLibDataStream::WriteLine(const char* line)
{
	FileSz res = Write(line);
	return res + Write("\r\n");
}

FileSz StdLibDataStream::Read(char* buffer, FileSz size)
{
	file.read(buffer, size);
	return size;
}

FileSz StdLibDataStream::WritePos()
{
	return file.tellp();
}

FileSz StdLibDataStream::ReadPos()
{
	return file.tellg();
}

char* StdLibDataStream::ReadAll()
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

FileSz StdLibDataStream::Seek(FileSz position)
{
	//clamp position under file size
	FileSz finalPos = (position > fileSize) ? fileSize : position;
	//try to move the input ("get") pointer
	//and the output ("put") pointer to the same position
	file.seekg(finalPos);
	file.seekp(finalPos);
	
	//if they're both at the same position, this will be 0
	return file.tellp() - file.tellg();
}

inline void StdLibDataStream::updateFileSize()
{
	using namespace std;
	FileSz openPos = file.tellg(); //remember where in the file we started
	file.seekg(0, ios::end); //move to 0 chars away from file's end
	fileSize = file.tellg();
	//cout << "size: " << fileSize << "\n";
	file.seekg(0, ios::beg); //return to start
}
