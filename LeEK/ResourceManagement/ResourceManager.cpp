#include "ResourceManager.h"
#include "ResourceLoaders.h"
#include "DebugUtils/Assertions.h"
#include "Math/MathFunctions.h"
#include "Constants/AllocTypes.h"
#include "Strings/StringUtils.h"
#include "FileManagement/Filesystem.h"
#include "ResourceManagement/ResourceArchive.h"

using namespace LeEK;

TypedHandle<ResourceManager> resMgrHnd = 0;

ResourceManager::ResourceManager()
{
	cacheMax = 0;
	cacheUsed = 0;
}

ResourceManager::~ResourceManager(void)
{
}

bool ResourceManager::makeRoom(FileSz size)
{
	//sanity check!
	if(size > cacheMax)
	{
		return false;
	}
	//now try releasing resources
	while((cacheMax - cacheUsed) < size)
	{
		//there's no more resources being tracked, but there's still not enough room.
		//must be some shared_ptrs to those resources remaining elsewhere.
		if(lruList.empty())
		{
			return false;
		}

		freeOneResource();
	}
	return true;
}

char* ResourceManager::allocate(FileSz size)
{
	//ensure the buffer will fit in the cache
	if(!makeRoom(size))
	{
		return NULL;
	}

	char* res = CustomArrayNew<char>(size, RESFILE_ALLOC, "CacheBufAlloc");
	//null out this data!
	memset(res, 0, size);
	if(res)
	{
		cacheUsed += size;
	}
	return res;
}

void ResourceManager::freeOneResource()
{
	//pluck the resource at the END of the LRU.
	ResList::iterator toRemove = lruList.end();
	toRemove--;

	//get the pointer, so we can also remove the reference in the map.
	std::shared_ptr<Resource> resource = *toRemove;

	//and remove the respective pointers from the res collections
	lruList.pop_back();
	resMap.erase(resource->GUID().Name);
}

void ResourceManager::ReportMemoryFreed(FileSz sizeFreed)
{
	//notify the resource manager of the freed memory
	L_ASSERT(sizeFreed <= cacheUsed && "Attempted to free more memory than used by resource cache!");
	cacheUsed -= Math::Min(sizeFreed, cacheUsed);
}

std::shared_ptr<Resource> ResourceManager::find(ResGUID* resGUID)
{
	ResMap::const_iterator resIt = resMap.find(resGUID->Name);
	if(resIt != resMap.end())
	{
		return resIt->second;
	}
	return std::shared_ptr<Resource>();
}

const void* ResourceManager::update(std::shared_ptr<Resource> res)
{
	//search the list for the resource.
	ResList::iterator it = lruList.begin();
	while(it != lruList.end())
	{
		if((*it)->GUID() == res->GUID())
		{
			break;
		}
		++it;
	}
	//quit if it wasn't found.
	if(it == lruList.end())
	{
		return NULL;
	}
	//otherwise, pop it and push it to the front.
	else
	{
		lruList.erase(it);
		lruList.push_front(res);
		return res.get();
	}
}

bool ResourceManager::openResource(const ResGUID& resGUID, char** outRawBuf, FileSz* outRawSize, bool useRawResource)
{
	String archName = ToLower(resGUID.ResArchiveName());
	if(archiveMap.count(archName) < 1)
	{
		if(!OpenArchive(resGUID))
		{
			LogW(String("Couldn't load archive ") + archName + "!");
			*outRawBuf = NULL;
			*outRawSize = 0;
			return false;
		}
	}

	//now try to allocate a raw data buffer
	//it's temporary if the loader indicates to not use raw data
	TypedHandle<IResourceArchive> archive = archiveMap[archName];
	FileSz rawSize = archive->GetRawSize(resGUID);
	*outRawSize = rawSize;
	if(!rawSize)
	{
		*outRawBuf = NULL;
		return false;
	}
	*outRawBuf =	useRawResource ? 
					allocate(rawSize) : 
					CustomArrayNew<char>(rawSize, RESFILE_ALLOC, "TempBufAlloc");
	archive->GetRawResource(resGUID, *outRawBuf);
	return true;
}

std::shared_ptr<Resource> ResourceManager::load(const ResGUID& resGUID)
{
	std::shared_ptr<IResourceLoader> loader;
	std::shared_ptr<Resource> resource;
	//try to get a loader...
	for(ResLoaderList::iterator it = loaderList.begin(); it != loaderList.end(); ++it)
	{
		std::shared_ptr<IResourceLoader> possLoader = *it;
		if(StringUtils::WildcardMatch(resGUID.ResName(), possLoader->GetPattern().c_str()))
		{
			loader = possLoader;
			break;
		}
	}
	//fail if there is no loader
	if(!loader)
	{
		L_ASSERT(loader && "No default resource loader found!");
		return resource;
	}

	//open the archive if necessary
	char* rawBuf = NULL;
	FileSz rawSize = 0;
	if(!openResource(resGUID, &rawBuf, &rawSize, loader->UseRawResource()))
	{
		return resource;
	}

	//the actual buffer
	char* resBuf = NULL;
	FileSz size = 0;

	//do the loader stuff
	if(loader->UseRawResource())
	{
		resBuf = rawBuf;
		resource = GetSharedPtr(CustomNew<Resource>(RESFILE_ALLOC, "ResAlloc", 
			resGUID, resBuf, rawSize, resMgrHnd));
	}
	else
	{
		size = loader->GetLoadedResSize(rawBuf, rawSize);
		resBuf = allocate(size);
		if(!rawBuf || !resBuf)
		{
			//out of memory again!
			return std::shared_ptr<Resource>();
		}
		resource = GetSharedPtr(CustomNew<Resource>(RESFILE_ALLOC, "ResAlloc", 
			resGUID, resBuf, size, resMgrHnd));
		bool resLoaded = loader->LoadResource(rawBuf, rawSize, resource);
		CustomArrayDelete(rawBuf);

		if(!resLoaded)
		{
			//something went wrong with the load
			return std::shared_ptr<Resource>();
		}
	}

	//now insert the resource into the res collections
	if(resource)
	{
		lruList.push_front(resource);
		resMap[resGUID.Name] = resource;
	}

	return resource;
}

std::shared_ptr<StreamingResource> ResourceManager::loadStream(const ResGUID& resGUID)
{
	std::shared_ptr<IResourceLoader> loader = NULL;
	std::shared_ptr<StreamingResource> stream = NULL;
	//try to get a loader...
	for(ResLoaderList::iterator it = loaderList.begin(); it != loaderList.end(); ++it)
	{
		std::shared_ptr<IResourceLoader> possLoader = *it;
		if(StringUtils::WildcardMatch(resGUID.ResName(), possLoader->GetPattern().c_str()))
		{
			loader = possLoader;
			break;
		}
	}
	//fail if there is no loader
	if(!loader)
	{
		L_ASSERT(loader && "No default stream loader found!");
		return stream;
	}

	//open the archive if necessary
	char* rawBuf = NULL;
	FileSz rawSize = 0;
	if(!openResource(resGUID, &rawBuf, &rawSize, loader->UseRawResource()))
	{
		return stream;
	}

	//the actual buffer
	char* resBuf = NULL;
	FileSz size = 0;

	//do the loader stuff
	if(loader->UseRawResource())
	{
		resBuf = rawBuf;
		stream = GetSharedPtr(LNew(StreamingResource, RESFILE_ALLOC, "ResAlloc")( 
			resGUID, rawSize, resMgrHnd));
	}
	else
	{
		size = loader->GetLoadedResSize(rawBuf, rawSize);
		resBuf = allocate(size);
		if(!rawBuf || !resBuf)
		{
			//out of memory again!
			return std::shared_ptr<StreamingResource>();
		}
		stream = GetSharedPtr(CustomNew<StreamingResource>(RESFILE_ALLOC, "ResAlloc", 
			resGUID, resBuf, size, resMgrHnd));
		bool resLoaded = loader->LoadStreamResource(rawBuf, rawSize, stream);
		CustomArrayDelete(rawBuf);

		if(!resLoaded)
		{
			//something went wrong with the load
			return std::shared_ptr<StreamingResource>();
		}
	}

	//now insert the stream into the res collections
	if(stream)
	{
		streamMap[resGUID.Name] = stream;
	}

	return stream;
}

void ResourceManager::release(std::shared_ptr<Resource> res)
{
	//remove the resource from the LRU and the map.
	lruList.remove(res);
	resMap.erase(res->GUID().Name);
}

bool ResourceManager::Init(FileSz cacheSizeMb)
{
	cacheMax = cacheSizeMb * 1024 * 1024;
	cacheUsed = 0;

	//add default loaders to the manager.
	loaderList.push_front(GetSharedPtr(CustomNew<DefaultResourceLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
	loaderList.push_front(GetSharedPtr(CustomNew<ModelLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));

	resMgrHnd = HandleMgr::RegisterPtr(this);
	if(!resMgrHnd.GetHandle())
	{
		LogW("Couldn't get handle for ResourceManager!");
		return false;
	}

	return true;
}

void ResourceManager::Shutdown()
{
	//free all the resources
	while(!lruList.empty())
	{
		freeOneResource();
	}
	//unregister all loaders
	while(!loaderList.empty())
	{
		loaderList.pop_front();
	}
	//close all our archives!
	for(ArchiveMap::iterator it = archiveMap.begin(); it != archiveMap.end(); ++it)
	{
		HandleMgr::DeleteHandle(it->second);
	}
}

void ResourceManager::RegisterLoader(std::shared_ptr<IResourceLoader> loader)
{
	loaderList.push_front(loader);
}

bool ResourceManager::OpenArchive(const ResGUID& guid)
{
	Path archPath = String(Filesystem::GetProgDir()) + guid.ResArchiveName();
	//check the archive's not already loaded
	String convertedName = ToLower(guid.ResArchiveName());
	//try to open the arch,
	//add it to the map if successful
	if(archiveMap.count(convertedName) < 1)
	{
		TypedHandle<IResourceArchive> resArch = HandleMgr::RegisterPtr(CustomNew<ZipResArchive>(RESFILE_ALLOC, "ResArchiveAlloc", archPath)).GetHandle();
		if(!resArch->Open())
		{
			return false;
		}
		archiveMap[convertedName] = resArch;
		return true;
	}
	LogV(String(convertedName) + " is already loaded!");
	return true;
}

std::shared_ptr<Resource> ResourceManager::GetResource(const ResGUID& guid)
{
	if(guid.Empty())
	{
		return NULL;
	}
	//get the resource if it's already loaded,
	//otherwise load from disk
	ResMap::iterator resIt = resMap.find(guid.Name);
	if(resIt != resMap.end())
	{
		update(resIt->second);
		return resIt->second;
	}
	return load(guid);
}

std::shared_ptr<StreamingResource> ResourceManager::GetStreamingResource(const ResGUID& guid)
{
	if(guid.Empty())
	{
		return NULL;
	}
	//get the resource if it's already loaded,
	//otherwise load from disk
	StreamResMap::iterator streamIt = streamMap.find(guid.Name);
	if(streamIt != streamMap.end())
	{
		//update(streamIt->second);
		return streamIt->second;
	}
	return loadStream(guid);
}

//GetStreamingResource?

//Preload

//Flush

//EditorResManager
TypedHandle<IResourceArchive> EditorResManager::findArchive(const ResGUID& archGUID)
{
	//check the archive's not already loaded
	String convertedName = ToLower(archGUID.ResArchiveName());
	ArchiveMap::iterator it = archiveMap.find(convertedName);
	TypedHandle<IResourceArchive> archHandle;
	if(it == archiveMap.end())
	{
		if(!OpenArchive(archGUID))
		{
			return 0;
		}
		archHandle = archiveMap[convertedName];
	}
	else
	{
		archHandle = it->second;
	}
	return archHandle;
}

U32 EditorResManager::GetNumResourcesInArchive(const ResGUID& archGUID)
{
	TypedHandle<IResourceArchive> archHandle = findArchive(archGUID);
	if(archHandle.GetHandle())
	{
		return archHandle->GetNumResources();
	}
	return 0;
}

ResGUID EditorResManager::GetResGUID(const ResGUID& archGUID, U32 resIndex)
{
	TypedHandle<IResourceArchive> archHandle = findArchive(archGUID);
	if(archHandle.GetHandle())
	{
		if(resIndex < archHandle->GetNumResources())
		{
			return ResGUID(archGUID.ResArchiveName(), archHandle->GetResourceName(resIndex));
		}
	}
	return ResGUID();
}

ResPtr EditorResManager::GetResourceFromIndex(const ResGUID& archGUID, U32 resIndex)
{ 
	return GetResource(GetResGUID(archGUID, resIndex));
}

//DebugResManager
bool DebugResManager::openResource(const ResGUID& resGUID, char** outRawBuf, FileSz* outRawSize, bool useRawResource)
{
	String archName = ToLower(resGUID.ResArchiveName());
	//Check the resource name.
	//If it indicates to use the filesystem, then use the filesystem!
	bool useFS = archName.empty();
	String filePath = Filesystem::GetProgDir() + String(resGUID.ResName());
	if(!useFS && archiveMap.count(archName) < 1)
	{
		if(!OpenArchive(resGUID))
		{
			LogW(String("Couldn't load archive ") + archName + "!");
			*outRawBuf = NULL;
			*outRawSize = 0;
			return false;
		}
	}

	//now try to allocate a raw data buffer
	//it's temporary if the loader indicates to not use raw data
	TypedHandle<IResourceArchive> archive = useFS ? 0 : archiveMap[archName];
	U32 rawSize = useFS ? Filesystem::GetFileSize(filePath) : archive->GetRawSize(resGUID);
	*outRawSize = rawSize;
	if(!rawSize)
	{
		*outRawBuf = NULL;
		return false;
	}
	*outRawBuf =	useRawResource ? 
					allocate(rawSize) : 
					CustomArrayNew<char>(rawSize, RESFILE_ALLOC, "TempBufAlloc");
	if(useFS)
	{
		auto file = Filesystem::OpenFileReadOnly(filePath);
		//TODO: make a CopyAll method in DataStream.
		auto buf = file->ReadAll();
		memcpy(*outRawBuf, buf, rawSize);
		Filesystem::CloseFile(file);
	}
	else
	{
		archive->GetRawResource(resGUID, *outRawBuf);
	}
	return true;
}

std::shared_ptr<Resource> DebugResManager::OpenResFromFile(const Path& path)
{
	return GetResource(ResGUID("",path.ToString()));
}