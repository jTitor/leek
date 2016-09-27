#pragma once
#include "DataStream.h"
#include "FileManagement/IStrStream.h"
#include <iostream>
#include <fstream>

namespace LeEK
{
	class StdLibDataStream : IStrStream
	{
	private:
		std::fstream file;
		FileSz fileSize;
		void updateFileSize();
	public:
		StdLibDataStream(void) {}
		~StdLibDataStream(void) {}

		inline FileSz FileSize() { return fileSize; }
		bool Open(const char* path, StreamFlag flags, IAsyncCallback* asyncCallback = NULL);
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