#include "ResourceLoaders.h"
#include "DataStructures/STLStreams.h"
#include "Rendering/Texture.h"
#include "Logging/Log.h"
#include "Rendering/Model.h"
#include "FileManagement/ModelFile.h"
#include <Libraries/PNG++/png.hpp>

using namespace LeEK;

//TODO: edit to use extra data system
//Also have resources autoload into rendering system as needed?
//Would also need them to be able to unload automatically

FileSz PNGLoader::GetLoadedResSize(char* rawBuf, FileSz rawSize)
{
	MemBuf wrappedBuf(rawBuf, rawSize);
	std::istream bufStr (&wrappedBuf);
	png::image<png::rgba_pixel> img(bufStr);
	//include a Texture2D struct?
	FileSz pixDataSize = img.get_height() * img.get_width() * 4;
	if(pixDataSize == 0)
	{
		return 0;
	}
	return sizeof(Texture2D) + pixDataSize;
}

bool PNGLoader::LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource)
{
	//wrap the raw data into a stream for PNG++ to use.
	MemBuf wrappedBuf(rawBuf, rawSize);
	std::istream bufStr (&wrappedBuf);
	png::image<png::rgba_pixel> img(bufStr);
	//Also init the texture header in the resource buffer.
	char* resBuf = resource->WriteableBuffer();
	Texture2D* texHeader = (Texture2D*)(void*)resBuf;
	//clear out the texture header.
	memset(texHeader, 0, sizeof(Texture2D));
	//the start of the buffer is a texture header.
	char* resBufData = resBuf + sizeof(Texture2D);
	texHeader->BitDepth = 32;
	texHeader->Data = resBufData;
	texHeader->HasMipMap = false;
	texHeader->PixType = Texture2D::RGBA8;
	texHeader->CompType = Texture2D::NONE;
	FileSz width = img.get_width();
	FileSz height = img.get_height();
	if(!width || !height)
	{
		return false;
	}
	texHeader->Width = width;
	texHeader->Height = height;

	//and copy the unpacked PNG data
	for(FileSz i = 0; i < height; ++i)
	{
		for(FileSz j = 0; j < width; ++j)
		{
			FileSz bytePos = (i*width*4) + (j * 4);
			//PNG pixels aren't stored in the same order as OpenGL expects;
			//you'll need to store rows in reverse order
			png::rgba_pixel pix = img.get_pixel(j, (height - i - 1));
			resBufData[bytePos] = pix.red;
			resBufData[bytePos + 1] = pix.green;
			resBufData[bytePos + 2] = pix.blue;
			resBufData[bytePos + 3] = pix.alpha;
		}
	}
	return true;
}

FileSz TGALoader::GetLoadedResSize(char* rawBuf, FileSz rawSize)
{
	Log::W("Calling unimplemented function!");
	return 0;
}

bool TGALoader::LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource)
{
	Log::W("Calling unimplemented function!");
	return false;
}

FileSz ModelLoader::GetLoadedResSize(char* rawBuf, FileSz rawSize)
{
	//Model needs to store the raw geometry data and a model object so the data can be used by the engine.
	if(!loadedSize)
	{
		loadedSize = sizeof(Model) + ModelMgr::FindFileGeomSize(rawBuf);
	}
	return loadedSize;
}

U16 getMeshCount(char* rawBuf)
{
	if(!rawBuf)
	{
		return 0;
	}

	//The start should be a main header.
	const ModelHeader* mainHdr = (ModelHeader*)rawBuf;
	//verify file integrity
	if(mainHdr->Signature != ModelHeader::SIGNATURE)
	{
		LogE("Couldn't verify model header!");
		return 0;
	}

	return mainHdr->NumUnits;
}

bool ModelLoader::LoadResource(char* rawBuf, FileSz rawSize, std::shared_ptr<Resource> resource)
{
	//need to do friggin' placement new?
	Model* model = new ((Model*)resource->WriteableBuffer()) Model(getMeshCount(rawBuf));
	char* geomStart = resource->WriteableBuffer() + sizeof(Model);
	size_t geomSize = GetLoadedResSize(rawBuf, rawSize) - sizeof(Model);
	bool result = ModelMgr::ReadModelMemory(*model, rawBuf, rawSize, geomStart, geomSize);
	return result;
}