using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ModelTool.Statics
{
    /**
     * Handles functions exposed by the ModelImporterLibrary DLL.
     */
    static class NativeFunctions
    {
        const string importerDLLName = "ModelImporterLibrary.dll";

        /*
         * General Information:
         *  * All functions in the DLL should have C linkage.
		 *  * Anything returning an IntPtr should be considered const.
         */

		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern uint GetLogBufferLen();
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.BStr)]
		public static extern string GetLogMsg(uint idx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetLogMsgLen(uint idx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetLogMsgVerbosity(uint idx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void ListModelData(int modelIdx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void ClearLogBuffer();

        [DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int ImportFromBuffer(IntPtr data, int dataSize);

        //Since strings are managed data, you need to marshal the parameter.
        [DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.I1)]
        public static extern bool 
            ExportCurrentModel([MarshalAs(UnmanagedType.BStr)]string outPath);

		//Export operations:
		/**
		Gets the size the given model would be when exported.
		*/
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetModelExportedSize(int modelIdx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetNumModelsLoaded();

		/**
		Exports the current model to a buffer of data.
		The given buffer must be at least as large
		as the value reported by GetModelExportedSize.
		*/
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool ExportModel(int modelIdx, IntPtr outBuffer, int bufSize);

		//Model properties:
		//IntPtrs are float*
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern bool GetModelAABBBounds(int modelIndx, IntPtr outBuf);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern bool GetModelCenter(int modelIndx, IntPtr outBuf);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern float GetModelRadius(int modelIndx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetMeshCount(int modelIndx);

		//Mesh properties
		//Geometry properties
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.I4)]
		public static extern int GetMeshVertexCount(int modelIndx, int meshIndx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.I4)]
		public static extern int GetMeshIndexCount(int modelIndx, int meshIndx);

		//Material properties
		//These return float*.
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		//[return: MarshalAs(UnmanagedType.)]
		public static extern IntPtr GetMeshMaterialDiffuseColor(int modelIndx, int meshIndx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr GetMeshMaterialSpecularColor(int modelIndx, int meshIndx);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr GetMeshMaterialEmissiveColor(int modelIndx, int meshIndx);

		//char*
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.BStr)]
		public static extern string GetMeshMaterialDiffuseMapGUID(int modelIndx, int meshIndx);

		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.BStr)]
		public static extern string GetMeshMaterialSpecularMapGUID(int modelIndx, int meshIndx);

		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.BStr)]
		public static extern string GetMeshMaterialEmissiveMapGUID(int modelIndx, int meshIndx);

		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.BStr)]
		public static extern string GetMeshMaterialNormalMapGUID(int modelIndx, int meshIndx);

		//Rendering methods
		//HWNDs need to be passed as an IntPtr.
		//Makes sense, since they're just void*.
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr SetViewPort(IntPtr vPort);

		//Previewer methods
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void InitPreviewer();
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void SpinCamera(float xEuler, float yEuler);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void ZoomCamera(float zoomDist);
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void Draw();
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetInputTargetHWND(IntPtr target);
		//Called whenever a message is passed to the window using this library,
		//allowing any message loop used by the library to intercept messages.
		[DllImport(importerDLLName, CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr WndProc(IntPtr hwnd, Int32 umessage, IntPtr wparam, IntPtr lparam);
    }
}
