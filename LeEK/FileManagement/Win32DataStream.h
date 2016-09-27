#pragma once
#ifdef WIN32
#include "DataStream.h"
#include "FileManagement/IStrStream.h"
#include <Windows.h>

namespace LeEK
{
	//Context that gets passed to an asynchronous IO stream.
	struct StreamContext
	{
		wchar_t* Path;
		StreamFlag Flags;
		AsyncRequest AsyncType;
		IAsyncCallback* Callbacks;
		/**
		The number of bytes read in a single read call.
		This should be a power of 2, and larger than 4096
		(a common disk sector size).
		*/
		FileSz ChunkSize;
		/**
		Number of chunks held by this stream at any given time.
		This is here mostly for audio streaming.
		*/
		FileSz NumChunks;
		//Following are used by write operations.
		char* Buffer;
		FileSz BufferSz;
	};

	class Win32DataStream : IStrStream
	{
	private:
		wchar_t* path;
		StreamFlag flags;
		OVERLAPPED ovrLp;
		HANDLE fileHnd;
		FileSz fileSize;
		void reset();
		void updateFileSize();
		FileSz getFilePos();

		//Members for asynchronous IO.
		bool isOverlapped;
		//HANDLE asyncThreadHnd;
		DWORD asyncThreadID;
		//HANDLE abortStreamEvt;

		//bool openAsync(const char* path, StreamFlag flags, IAsyncCallback* asyncCallback);

		//For now, forbid copy and assignment.
		Win32DataStream(const Win32DataStream& copy);
		Win32DataStream& operator=(const Win32DataStream& copy);
	public:
		Win32DataStream(const char* pPath, StreamFlag pFlags);
		~Win32DataStream();

		inline FileSz FileSize() { return fileSize; }
		bool Open(const char* pPath, StreamFlag pFlags);
		bool DoAsync(AsyncRequest asyncType, char* buffer, FileSz bufSz, IAsyncCallback* asyncCallback);
		/*
		bool Open(const char* path);
		bool OpenText(const char* path);
		bool OpenReadOnly(const char* path);
		*/
		bool Close();
		FileSz Write(const char* buffer, FileSz size);
		FileSz Write(const char* buffer);
		FileSz WriteLine(const char* line);
		FileSz WriteLine(const String& line) { return WriteLine(line.c_str()); }
		FileSz Read(char* buffer, FileSz size);
		FileSz WritePos();
		FileSz ReadPos();
		char* ReadAll();
		FileSz Seek(FileSz position);
	};
}
#endif //WIN32