#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#pragma once
#include "Strings/String.h"
#include "Datatypes.h"
#include "path.h"
#include "Platforms/IPlatform.h"

namespace LeEK
{
	class DataStream;
	class IPlatform;

	namespace Filesystem
	{
		void SetPlatform(IPlatform* val);
		const IPlatform* GetPlatform();
		bool PlatformSet();

		//provide overloads for using Path parameters of some kind?

		//path properties
		bool Exists(const Path& p);
		bool IsEmpty(const Path& p);
		/**
		*	Returns the current working path -
		*	NOT the same as the executable's directory.
		*/
		const char* GetCurrentPath();
		/**
		*	Returns the full directory path to the executable.
		*/
		const char* GetProgDir();

		//path operations
		bool MakeDirectory(const Path& p);
		void Rename(const Path& oldP, const Path& newP);
		bool RemoveFile(const Path& p);

		//size functions, should return in bytes
		//consider changing return value? can only describe a max file size of ~4 GB
		FileSz GetFileSize(const Path& p);
		U64 FindFreeSpace(); //will get free space in current working directory

		//file opening functions
		//Opens a file with read/write permissions, 
		//placing the current position at the beginning of the file.
		DataStream* OpenFile(const Path& p);
		DataStream* OpenFileText(const Path& p);
		DataStream* OpenFileReadOnly(const Path& p);
		AsyncDataStream* OpenFileAsync(const Path& p);
		bool CloseFile(DataStream* stream);
	}
}
#endif