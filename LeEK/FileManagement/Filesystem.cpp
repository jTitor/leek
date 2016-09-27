#include "StdAfx.h"
#include "Filesystem.h"
#include "StdLibDataStream.h"
#include "Constants/LogTags.h"
#include <boost/filesystem.hpp>

using namespace LeEK;

IPlatform* plat = NULL;

void Filesystem::SetPlatform(IPlatform* val)
{
	plat = val;
}

bool Filesystem::PlatformSet()
{
	return plat != NULL;
}

const IPlatform* Filesystem::GetPlatform()
{
	return plat;
}

bool Filesystem::Exists(const Path& p)
{
	return boost::filesystem::exists(p.PathImplementation());
}

bool Filesystem::IsEmpty(const Path& p)
{
	return boost::filesystem::is_empty(p.PathImplementation());
}

const char* Filesystem::GetCurrentPath()
{
	return (char*)boost::filesystem::current_path().c_str();
}

const char* Filesystem::GetProgDir()
{
	if(!plat)
	{
		LogE("Filesystem: no platform attached!");
		return "";
	}
	return plat->GetProgDir();
}

//path operations
bool Filesystem::MakeDirectory(const Path& p)
{
	return boost::filesystem::create_directories(p.PathImplementation()) || boost::filesystem::exists(p.PathImplementation());
}

void Filesystem::Rename(const Path& oldP, const Path& newP)
{
	boost::filesystem::rename(oldP.PathImplementation(), newP.PathImplementation());
}
bool Filesystem::RemoveFile(const Path& p)
{
	return boost::filesystem::remove(p.PathImplementation());
}

//size functions, should return in bytes
//consider changing return value? can only describe a max file size of ~4 GB
FileSz Filesystem::GetFileSize(const Path& p)
{
	return boost::filesystem::file_size(p.PathImplementation());
}

U64 Filesystem::FindFreeSpace()
{
	return boost::filesystem::space(boost::filesystem::current_path()).available;
}

//file opening functions
DataStream* Filesystem::OpenFile(const Path& p)
{
	DataStream* ds;
	//try to open the best possible data stream
	ds = (DataStream*)CustomNew<StdLibDataStream>(FILESYS_ALLOC, LogTags::FILESYS_ALLOC);

	if(!ds->Open(p.ToString().c_str()))
	{
		return NULL;
	}
	return ds;
}

DataStream* Filesystem::OpenFileText(const Path& p)
{
	DataStream* ds;
	//try to open the best possible data stream
	ds = (DataStream*)CustomNew<StdLibDataStream>(FILESYS_ALLOC, LogTags::FILESYS_ALLOC);

	if(!ds->OpenText(p.ToString().c_str()))
	{
		return NULL;
	}
	return ds;
}

DataStream* Filesystem::OpenFileReadOnly(const Path& p)
{
	DataStream* ds;
	//try to open the best possible data stream
	ds = (DataStream*)CustomNew<StdLibDataStream>(FILESYS_ALLOC, LogTags::FILESYS_ALLOC);

	if(!ds->OpenReadOnly(p.ToString().c_str()))
	{
		return NULL;
	}
	return ds;
}

//Note that the stream is invalid from here on out!
bool Filesystem::CloseFile(DataStream* stream)
{
	if(stream)
	{
		bool result = stream->Close();
		CustomDelete(stream);
		stream = NULL;
		return result;
	}
	return true;
}