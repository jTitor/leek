#pragma once
#include "IResourceLoader.h"
#include "Rendering/Texture.h"

namespace LeEK
{
	class DefaultResourceLoader : public IResourceLoader
	{
	public:
		//should match anything
		virtual String GetPattern() { return "*"; }
		//does no processing, so just use raw data
		virtual bool UseRawResource() { return true; }
		virtual FileSz GetLoadedResSize(char* rawBuf, FileSz rawSize) { return rawSize; }
		//does no processing
		virtual bool LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource) { return true; }
		virtual FileSz StreamRead(DataStream* pData, char* dest, FileSz numBytes);
		virtual FileSz StreamSeek(DataStream* pData, FileSz relOffset);
		virtual FileSz StreamClose();
	};

	//For image loaders, the front of the loaded resource can be read as a texture.
	class PNGLoader : public IResourceLoader
	{
	public:
		virtual String GetPattern() { return "*.png"; }
		virtual bool UseRawResource() { return false; }
		virtual FileSz GetLoadedResSize(char* rawBuf, FileSz rawSize);
		virtual bool LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource);
	};

	class TGALoader : public IResourceLoader
	{
	public:
		virtual String GetPattern() { return "*.tga"; }
		virtual bool UseRawResource() { return false; }
		virtual FileSz GetLoadedResSize(char* rawBuf, FileSz rawSize);
		virtual bool LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource);
	};

	//the front of the loaded resource can be read as a Model object.
	class ModelLoader : public IResourceLoader
	{
	private:
		size_t loadedSize;
	public:
		ModelLoader() : loadedSize(0) {}
		virtual String GetPattern() { return "*.lmdl"; }
		virtual bool UseRawResource() { return false; }
		virtual FileSz GetLoadedResSize(char* rawBuf, FileSz rawSize);
		virtual bool LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource);
	};
}