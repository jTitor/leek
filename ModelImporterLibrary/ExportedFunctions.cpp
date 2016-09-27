#include "ExportedFunctions.h"
#include <FileManagement/ModelFile.h>
#include <Strings/StringUtils.h>
#include <Platforms/Win32Platform.h>
#include <Platforms/Win32Helpers.h>
#include "ModelConversion.h"
#include "ModelLog.h"
#include "ConverterManager.h"
#include "ModelPreview.h"
#include "Devices.h"

using namespace LeEK;

Geometry invalidGeom = Geometry();
Material invalidMat = Material();

unsigned int GetLogBufferLen()
{
	return ModelLog::BufferLength();
}

BSTR GetLogMsg(unsigned int idx)
{
	string msg = ModelLog::GetMsg(idx);
	return Win32Helpers::MultiToBSTR(msg);
}

int GetLogMsgLen(unsigned int idx)
{
	return ModelLog::GetMsg(idx).length();
}

int GetLogMsgVerbosity(unsigned int idx)
{
	return (int)ModelLog::GetMsgVerbosity(idx);
}

void ListModelData(int modelIdx)
{
	int numVert = 0;
	int numInd = 0;
	Model& mdl = ConverterManager::GetModel(modelIdx);
	LogV("Listing model data:");
	ModelLog::V("\tBounds Center: "+mdl.BoundsCenter().ToString());
	ModelLog::V(string("\tBounds Radius: ")+mdl.BoundingRadius());
	ModelLog::V("\tAABB Bounds: "+mdl.AABBBounds().ToString());
	ModelLog::V(string("\tNum. Meshes: ")+mdl.MeshCount());
	for(int i = 0; i < mdl.MeshCount(); ++i)
	{
		Mesh* mesh = mdl.GetMesh(i);
		if(mesh != NULL)
		{
			ModelLog::V(string("\tMesh ") + i + ":");
			Geometry& geom = mesh->GetGeometry();
			ModelLog::V(string("\t\tNum. Vertices: ") + geom.VertexCount());
			ModelLog::V(string("\t\tNum. Indices: ") + geom.IndexCount());
			numVert += geom.VertexCount();
			numInd += geom.IndexCount();
			Material& mat = mesh->GetMaterial();
			ModelLog::V("\t\tDiffuse Color: " + mat.DiffuseColor.ToString());
			ModelLog::V("\t\tSpecular Color: " + mat.SpecularColor.ToString());
			ModelLog::V("\t\tEmissive Color: " + mat.EmissiveColor.ToString());
			ModelLog::V(string("\t\tDiffuse GUID: ") + mat.DiffuseTexGUID.Name);
			ModelLog::V(string("\t\tSpecular GUID: ") + mat.SpecularTexGUID.Name);
			ModelLog::V(string("\t\tEmissive GUID: ") + mat.EmissiveTexGUID.Name);
			ModelLog::V(string("\t\tNormal Map GUID: ") + mat.NormalMapGUID.Name);
		}
	}
	ModelLog::V(string("\tTotal vertex count: ") + numVert);
	ModelLog::V(string("\tTotal index count: ") + numInd);
}

void ClearLogBuffer()
{
	return ModelLog::ForceDumpBuffer();
}

int ImportFromBuffer(char* data, int dataSize)
{
	return ConverterManager::ImportModel(data, dataSize);
}

int LoadFromBuffer(char* data, int dataSize)
{
	return ConverterManager::LoadModel(data, dataSize);
}

//int ImportFromFile(BSTR inPath)
//{
//}

int GetModelExportedSize(int modelIdx)
{
	return ConverterManager::GetModelConvertedSize(modelIdx);
}

int GetNumModelsLoaded()
{
	return ConverterManager::GetNumModels();
}

//Export operations:
bool ExportModel(int modelIdx, char* outBuffer, int bufSize)
{
	return ModelMgr::WriteModelMemory(ConverterManager::GetModel(modelIdx), outBuffer, bufSize);
}

//Model properties:
const int GetModelAABBBounds(int modelIndx, float* out)
{
	const float* resBuf = ConverterManager::GetModel(modelIndx).AABBBounds().ToFloatArray();
	int numBytes = sizeof(float) * 3;
	memcpy_s(out, numBytes, resBuf, numBytes);
	return numBytes;
}
const int GetModelCenter(int modelIndx, float* out)
{
	const float* resBuf = ConverterManager::GetModel(modelIndx).BoundsCenter().ToFloatArray();
	int numBytes = sizeof(float) * 3;
	memcpy_s(out, numBytes, resBuf, numBytes);
	return numBytes;
}
float GetModelRadius(int modelIndx)
{
	return ConverterManager::GetModel(modelIndx).BoundingRadius();
}
int GetMeshCount(int modelIndx)
{
	return ConverterManager::GetModel(modelIndx).MeshCount();
}

Geometry& getGeom(int modelIndx, int meshIndx)
{
	Model& mdl = ConverterManager::GetModel(modelIndx);
	if(meshIndx < 0 || meshIndx >= mdl.MeshCount())
	{
		return invalidGeom;
	}
	return mdl.GetMesh(meshIndx)->GetGeometry();
}

Material& getMaterial(int modelIndx, int meshIndx)
{
	Model& mdl = ConverterManager::GetModel(modelIndx);
	if(meshIndx < 0 || meshIndx >= mdl.MeshCount())
	{
		return invalidMat;
	}
	return mdl.GetMesh(meshIndx)->GetMaterial();
}

//Mesh properties
//Geometry properties
int GetMeshVertexCount(int modelIndx, int meshIndx)
{
	return getGeom(modelIndx, meshIndx).VertexCount();
}
int GetMeshIndexCount(int modelIndx, int meshIndx)
{
	return getGeom(modelIndx, meshIndx).IndexCount();
}

//Material properties
const float* GetMeshMaterialDiffuseColor(int modelIndx, int meshIndx)
{
	return getMaterial(modelIndx, meshIndx).DiffuseColor.ToFloatArray();
}
const float* GetMeshMaterialSpecularColor(int modelIndx, int meshIndx)
{
	return getMaterial(modelIndx, meshIndx).SpecularColor.ToFloatArray();
}
const float* GetMeshMaterialEmissiveColor(int modelIndx, int meshIndx)
{
	return getMaterial(modelIndx, meshIndx).EmissiveColor.ToFloatArray();
}

BSTR GetMeshMaterialDiffuseMapGUID(int modelIndx, int meshIndx)
{
	char* guid = getMaterial(modelIndx, meshIndx).DiffuseTexGUID.Name;
	return Win32Helpers::MultiToBSTR(guid, strlen(guid));
}
BSTR GetMeshMaterialSpecularMapGUID(int modelIndx, int meshIndx)
{
	char* guid = getMaterial(modelIndx, meshIndx).SpecularTexGUID.Name;
return Win32Helpers::MultiToBSTR(guid, strlen(guid));
}
BSTR GetMeshMaterialEmissiveMapGUID(int modelIndx, int meshIndx)
{
	char* guid = getMaterial(modelIndx, meshIndx).EmissiveTexGUID.Name;
return Win32Helpers::MultiToBSTR(guid, strlen(guid));
}
BSTR GetMeshMaterialNormalMapGUID(int modelIndx, int meshIndx)
{
	char* guid = getMaterial(modelIndx, meshIndx).NormalMapGUID.Name;
return Win32Helpers::MultiToBSTR(guid, strlen(guid));
}

void SetViewPort(HWND vPort)
{
	Previewer::GetInstance()->SetViewport(vPort);
}

void InitPreviewer()
{
	Previewer::GetInstance();
}
void SpinCamera(float xEuler, float yEuler)
{
	Previewer::GetInstance()->SpinCamera(xEuler, yEuler);
}
void ZoomCamera(float zoomDist)
{
	Previewer::GetInstance()->ZoomCamera(zoomDist);
}
void Draw()
{
	auto plat = Devices::GetPlatform();
	auto gfx = Devices::GetGfx();
	if(!gfx)
	{
		return;
	}
	plat->BeginRender(gfx);
	Previewer::GetInstance()->Draw();
	plat->EndRender(gfx);
}

DllCExport void SetInputTargetHWND(HWND target)
{
	auto plat = Devices::GetPlatform();
	auto input = Devices::GetInput();
	if(!plat)
	{
		return;
	}
	plat->InitInput((WindowHnd)target, input);
}

LRESULT WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	return Win32Platform::WindowProc(hwnd, umessage, wparam, lparam);
}