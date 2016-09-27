#include "ResourceArchive.h"

using namespace LeEK;

ZipResArchive::ZipResArchive(const Path& archivePath) : archPath(archivePath)
{
}

bool ZipResArchive::Open()
{
	if(!file.Init(archPath))
	{
		return false;
	}
	return true;
}

FileSz ZipResArchive::GetRawSize(const ResGUID& resource) const
{
	return file.GetFileLen(file.Find(resource.ResName()));
}

U32 ZipResArchive::GetRawResource(const ResGUID& resource, char* buffer)
{
	if(file.ReadFile(file.Find(resource.ResName()), buffer))
	{
		return GetRawSize(resource);
	}
	return 0;
}

U32 ZipResArchive::GetNumResources() const
{
	return (U32)file.GetNumFiles();
}

const String ZipResArchive::GetResourceName(U32 resNum) const
{
	return file.GetFilename(resNum);
}

const String ZipResArchive::GetArchiveName() const
{
	return archPath.GetBaseName();
}