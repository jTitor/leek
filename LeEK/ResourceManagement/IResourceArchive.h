#pragma once
#include "Datatypes.h"
#include "Strings/String.h"
#include "Memory/Handle.h"
#include "ResourceManagement/Resource.h"
#include "Logging/Log.h"

namespace LeEK
{
	class IResourceArchive
	{
	public:
		virtual ~IResourceArchive(void) {}
		virtual U32 GetRawSize(const ResGUID& resource) const = 0;
		virtual U32 GetRawResource(const ResGUID& resource, char* buffer) = 0;
		virtual U32 GetNumResources() const = 0;
		virtual const String GetResourceName(U32 resNum) const = 0;
		virtual const String GetArchiveName() const = 0;
		virtual bool Open() = 0;
	};
}
