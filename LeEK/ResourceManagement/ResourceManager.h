#pragma once
#include "Resource.h"
#include "IResourceLoader.h"
#include "IResourceArchive.h"
#include "DataStructures/STLContainers.h"
#include "FileManagement/path.h"

namespace LeEK
{
	typedef std::shared_ptr<Resource> ResPtr;

	class ResourceManager
	{
	protected:
		typedef List<std::shared_ptr<Resource>> ResList;
		typedef Map<String, std::shared_ptr<Resource>> ResMap;
		typedef Map<String, std::shared_ptr<StreamingResource>> StreamResMap;
		typedef Map<String, TypedHandle<IResourceArchive>> ArchiveMap;
		typedef List<std::shared_ptr<IResourceLoader>> ResLoaderList;

		//Least Recently Used list.
		//Stuff in front's most used, stuff in back least.
		ResList lruList;
		ResMap resMap;
		StreamResMap streamMap;
		ArchiveMap archiveMap;
		ResLoaderList loaderList;

		//both measured in bytes
		FileSz cacheMax;
		FileSz cacheUsed;

		//Returns true if enough room has been freed, false otherwise
		bool makeRoom(FileSz size);
		//Attempts to create a buffer of the desired size for the cache.
		char* allocate(FileSz size);
		void freeOneResource();

		//Resource managing methods:
		//Gets the resource corresponding to the GUID, if possible.
		std::shared_ptr<Resource> find(ResGUID* resGUID);
		//Notifies the manager that a resource has been used.
		const void* update(std::shared_ptr<Resource> res);
		//Actually loads resource data into memory.
		//Returns true if succesful, false otherwise.
		virtual bool openResource(const ResGUID& resGUID, char** outRawBuf, FileSz* outRawSize, bool useRawResource);
		//Loads a resource into memory.
		std::shared_ptr<Resource> load(const ResGUID& resGUID);
		std::shared_ptr<StreamingResource> loadStream(const ResGUID& resGUID);
		//Removes a resource from the manager.
		void release(std::shared_ptr<Resource> res);

	public:
		ResourceManager();
		virtual ~ResourceManager(void);

		virtual bool Init(FileSz cacheSizeMb);
		//close all files!
		void Shutdown();
		void RegisterLoader(std::shared_ptr<IResourceLoader> loader);

		bool OpenArchive(const ResGUID& guid);
		std::shared_ptr<Resource> GetResource(const ResGUID& guid);
		std::shared_ptr<StreamingResource> GetStreamingResource(const ResGUID& guid);
		void ReportMemoryFreed(FileSz sizeFreed);
		//no support for streaming yet
		I32 Preload(const String pattern, void(*progressCallback)(I32, bool&));
		//got no idea what this does!
		void Flush(void);
	};

	class EditorResManager : public ResourceManager
	{
	protected:
		TypedHandle<IResourceArchive> findArchive(const ResGUID& archGUID);
	public:
		U32 GetNumResourcesInArchive(const ResGUID& archGUID);
		ResGUID GetResGUID(const ResGUID& archGUID, U32 resIndex);
		ResPtr GetResourceFromIndex(const ResGUID& archGUID, U32 resIndex);
	};

	class DebugResManager : public ResourceManager
	{
	protected:
		bool openResource(const ResGUID& resGUID, char** outRawBuf, FileSz* outRawSize, bool useRawResource);
	public:
		std::shared_ptr<Resource> OpenResFromFile(const Path& path);
	};
}