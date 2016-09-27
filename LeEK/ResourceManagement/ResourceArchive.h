#pragma once
#include "IResourceArchive.h"
#include "FileManagement/path.h"
#include "FileManagement/ArchiveTypes.h"
#include "Logging/Log.h"

namespace LeEK
{
	class ZipResArchive : public IResourceArchive
	{
	private:
		Path archPath;
		ZipFile file;
	public:
		ZipResArchive(const Path& archivePath);
		~ZipResArchive(void) {}
		FileSz GetRawSize(const ResGUID& resource) const;
		U32 GetRawResource(const ResGUID& resource, char* buffer);
		U32 GetNumResources() const;
		const String GetResourceName(U32 resNum) const;
		const String GetArchiveName() const;
		bool Open();
	};
}

