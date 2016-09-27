#pragma once
#include "ResourceManagement/Resource.h"
#include "DataStructures/STLContainers.h"
#include "Memory/STLAllocHook.h"
#include "Strings/String.h"
#include "Memory/Handle.h"

namespace LeEK
{
	class IResourceLoader
	{
	public:
		virtual String GetPattern() = 0;
		virtual bool UseRawResource() = 0;
		virtual U32 GetLoadedResSize(char* rawBuf, U32 rawSize) = 0;
		virtual bool LoadResource(char* rawBuf, U32 rawSize, std::shared_ptr<Resource> resource) = 0;
		virtual bool LoadStreamResource(char* rawBuf, FileSz rawSize, std::shared_ptr<DecodableResource> resource) { return false; }
		virtual FileSz StreamRead(DecodableResource* res, char* dest, FileSz numBytes) { return 0; }
		virtual FileSz StreamSeek(DecodableResource* res, FileSz offset) { return 0; }
		virtual void StreamClose(DecodableResource* res) { return; }
	};
}