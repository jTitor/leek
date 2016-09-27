#pragma once
#include <Windows.h>

//In general, functions must be exported with C linkage
//so that the C# code can properly find the functions in the resulting DLL.
//
//C# also CANNOT accept static libraries - this means libraries meant for use in C#
//must be built as DLLs.
#define DllCExport extern "C" __declspec(dllexport)

/*
General advice:
	* Functions being exposed to C# should avoid returning bools;
	you will have to do some marshalling on the C# side if you wish to do so,
	since boolean values are implemented differently in the two languages.
	* Arrays will be received as IntPtrs in C# -
	to interpret them, you need to Marshal.Copy them over to a managed array.
	This will at least let you pass a Vector3!
*/

//Logging ops:
DllCExport unsigned int GetLogBufferLen();
DllCExport BSTR GetLogMsg(unsigned int idx);
DllCExport int GetLogMsgLen(unsigned int idx);
DllCExport void ListModelData(int modelIdx);
/**
Gets the verbosity level of the given message,
or verbosity level ERR if the index is invalid.
The levels and their corresponding integer values are:
	ERR		= 0,
	WARN	= 1,
	INFO	= 2,
	DEBUG	= 3,
	VERB	= 4
*/
DllCExport int GetLogMsgVerbosity(unsigned int idx);
DllCExport void ClearLogBuffer();

//Import operations:
/**
Converts the given buffer of data to a model, if possible.
Returns the model's index or -1 if the model could not be created.
*/
DllCExport int ImportFromBuffer(char* data, int dataSize);
/**
Loads a .lmdl file from a buffer.
returns the model's index or -1 if the model could not be created.
*/
DllCExport int LoadFromBuffer(char* data, int dataSize);
//DllCExport int ImportFromFile(BSTR inPath);

//Export operations:
/**
Gets the size the given model would be when exported.
*/
DllCExport int GetModelExportedSize(int modelIdx);
DllCExport int GetNumModelsLoaded();
/**
Exports the current model to a buffer of data.
The given buffer must be at least as large
as the value reported by GetModelExportedSize.
*/
DllCExport bool ExportModel(int modelIdx, char* outBuffer, int bufSize);

//Model properties:
//These will be used in a managed context,
//so the CLIENT passes in a buffer to fill with data.
//Returns the size of the buffer needed if out = NULL,
//the number of bytes written if out != NULL,
//and -1 if an error occurred.
DllCExport const int GetModelAABBBounds(int modelIndx, float* out);
DllCExport const int GetModelCenter(int modelIndx, float* out);
DllCExport float GetModelRadius(int modelIndx);
DllCExport int GetMeshCount(int modelIndx);
//Because of the preview system, should consider adding setters too!

//Mesh properties
//Geometry properties
DllCExport int GetMeshVertexCount(int modelIndx, int meshIndx);
DllCExport int GetMeshIndexCount(int modelIndx, int meshIndx);

//Material properties
DllCExport const float* GetMeshMaterialDiffuseColor(int modelIndx, int meshIndx);
DllCExport const float* GetMeshMaterialSpecularColor(int modelIndx, int meshIndx);
DllCExport const float* GetMeshMaterialEmissiveColor(int modelIndx, int meshIndx);

DllCExport BSTR GetMeshMaterialDiffuseMapGUID(int modelIndx, int meshIndx);
DllCExport BSTR GetMeshMaterialSpecularMapGUID(int modelIndx, int meshIndx);
DllCExport BSTR GetMeshMaterialEmissiveMapGUID(int modelIndx, int meshIndx);
DllCExport BSTR GetMeshMaterialNormalMapGUID(int modelIndx, int meshIndx);

//Rendering methods
DllCExport void SetViewPort(HWND vPort);

//Previewer methods
DllCExport void InitPreviewer();
DllCExport void SpinCamera(float xEuler, float yEuler);
DllCExport void ZoomCamera(float zoomDist);
DllCExport void Draw();

DllCExport void SetInputTargetHWND(HWND target);
//Called whenever a message is passed to the window using this library,
//allowing any message loop used by the library to intercept messages.
DllCExport LRESULT WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);