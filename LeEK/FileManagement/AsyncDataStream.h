#pragma once
#include "Datatypes.h"
#include "DataStream.h"
#include <cstdio>

namespace LeEK
{
	/**
	TODO: Refactor to IAsyncDataStream
	Interface for datastreams that asynchronously read/write data.
	*/
	class IAsyncDataStream : DataStream
	{
	public:
		virtual ~IAsyncDataStream(void) {}
		//virtual bool IsAsync() = 0;
		virtual bool OpenAsync(const char* path) = 0;
		virtual bool OpenTextAsync(const char* path) = 0;
		virtual bool OpenReadOnlyAsync(const char* path) = 0;
		virtual bool Close() = 0;
		virtual FileSz FileSize() = 0;
		virtual FileSz WriteAsync(const char* buffer, FileSz size) = 0;
		virtual FileSz ReadAsync(char* buffer, FileSz offset, FileSz size) = 0;
		virtual FileSz WritePos() = 0;
		virtual FileSz ReadPos() = 0;
		virtual char* ReadAll() = 0;
		virtual FileSz Seek(FileSz position) = 0; //move to (position) bytes from beginning of file
		virtual FileSz FileEnd() { return FileSize() - 1; }
		virtual FileSz SeekToEnd() { return Seek(FileSize()-1); }
		virtual FileSz SeekFromEnd(FileSz negPosition) { return Seek(FileSize() - negPosition); }
		//virtual FileSz WriteAsync(const U8* buffer, FileSz offset, FileSz count) = 0;
		//virtual FileSz ReadAsync(U8* buffer, FileSz offset, FileSz count) = 0;
		//virtual FileSz ReadByte() = 0;
	};
}