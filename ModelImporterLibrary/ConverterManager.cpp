#include "ConverterManager.h"
#include "ModelConversion.h"
#include <FileManagement/ModelFile.h>
#include <Rendering/Geometry.h>
#include <Time/GameTime.h>
#include "ModelPreview.h"

using namespace LeEK;

Geometry* geomList = NULL;
//CustomNew<GameTime>(AllocType::PLAT_ALLOC, "PlatAlloc");
Model model = Model();
U32 numMeshes = 0;
U32 numModelsLoaded = 0;
Previewer* previewer = NULL;
void onModelLoaded()
{
	if(!previewer)
	{
		previewer = Previewer::GetInstance();
	}
	previewer->NotifyModelChanged();
	//only one model is ever loaded
	numModelsLoaded = 1;
}

void onModelUnload()
{
	//only one model is ever loaded
	numModelsLoaded = 1;
}

ConverterManager::ConverterManager(void)
{
	previewer = Previewer::GetInstance();
}

ConverterManager::~ConverterManager(void)
{
}

Model& ConverterManager::GetModel(int modelIdx)
{
	return model;
}

int ConverterManager::GetNumModels()
{
	return numModelsLoaded;
}

int ConverterManager::GetModelConvertedSize(int modelIdx)
{
	return ModelMgr::FindFileSize(model);
}

int ConverterManager::ImportModel(char* data, size_t dataSize)
{
	Model result = Model();
	if(!ModelConversion::ImportModel(data, dataSize, geomList, result))
	{
		return -1;
	}
	model = result;
	onModelLoaded();
	return GetNumModels();
}

int ConverterManager::LoadModel(char* data, size_t dataSize)
{
	Model result = Model();
	size_t geomBufSize = ModelMgr::FindFileGeomSize(data);
	char* geomBuf = new char[geomBufSize];
	if(!ModelMgr::ReadModelMemory(result, data, dataSize, geomBuf, geomBufSize))
	{
		delete[] geomBuf;
		return -1;
	}
	delete[] geomBuf;
	onModelUnload();
	model = result;
	onModelLoaded();
	return GetNumModels();
}
