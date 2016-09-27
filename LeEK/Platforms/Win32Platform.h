#ifndef WIN32PLATFORM_H
#define WIN32PLATFORM_H
#pragma once
#ifdef WIN32
//Dunno why, but hidpi can't figure out what NTSTATUS evaluates to.
#include "Datatypes.h"
//typedef I32 NTSTATUS;
#include <Windows.h>
#include <hidusage.h>
#include <hidpi.h>
#include "IPlatform.h"
#include "GraphicsWrappers/OGLGrpWrapper.h"
#include "DataStructures/STLContainers.h"
#include "Audio/AudioManager.h"


namespace LeEK
{
	class Win32Platform : public IPlatform
	{
	private:
		//This is needed since WindowProc needs to be static, and of course a static function
		//can't call nonstatic fields;
		//in any case, there shouldn't be more than one engine window running per application.
		static Win32Platform* instance;

		//window constants
		const static U32 DEFAULT_WINDOW_WIDTH = 1024;
		const static U32 DEFAULT_WINDOW_HEIGHT = 768;
		U32 displayWidth;
		U32 displayHeight;
		U32 clientWidth;
		U32 clientHeight;

		LPCWSTR appName;
		HINSTANCE appInstance;
		HDC devContext;
		MSG msg;
		bool wndClassRegistered;
		HWND window;

		//OpenGL specific renderer data
		HGLRC renderContext;
		bool isFullscreen;
		bool functionsLoaded;

		//input data
		Handle inputMgrHnd;
		//preparsed data, indexed by device handle
		Map<U64, PHIDP_PREPARSED_DATA> preParDataMap;
		//HID count
		U8 numHID;
		//following arrays are indexed by device's index according to InputManager
		//general info array
		HIDP_CAPS* ctrlStats;
		//detailed info for axis and buttons.
		HIDP_BUTTON_CAPS** btnStats;
		HIDP_VALUE_CAPS** valStats;
		//buffer for device report

		bool oglLoadFunctions(OGLGrpWrapper* grpWrapper, HWND hwnd);
		bool oglInitRenderer(OGLGrpWrapper* grpWrapper, HWND hwnd, F32 farPlaneDist, F32 nearPlaneDist, bool vSync);
		bool loadInputFuncs();
		bool loadDLLs();

		bool registerWindowClass(WNDCLASSEX* pWC);

		/**
		Attempts to attach a renderer to the desired viewport.
		Returns true if no fatal errors occurred during the attachment.
		In particular, this means that this will return true if gfx is null.
		*/
		bool attachRenderer(GfxWrapperHandle gfx, HWND viewPort);
		/**
		Attempts to perform first-run initialization, such as function loading,
		on the given graphics wrapper. Returns true if 
		no fatal errors occurred during initialization. In particular,
		this means that this will return true if gfx is null.

		Should only be called once.
		*/
		bool firstInitRenderer(IGraphicsWrapper* gfx);

		bool onUpdateInput(WPARAM wPar, LPARAM lPar);
		void onUpdateGUI(LPARAM lPar);

		void buildHIDDetailList();
		void registerAllInputDevices(HWND targetWindow);
	protected:
		U8 getUniversalKeyCode(U8 keyCode);
		WindowHnd getWindow();
	public:
		Win32Platform(void);
		~Win32Platform(void);
		//constant accessors
		inline const PlatformType Type() { return Win32; }
		inline const char* FilesystemPathSeparator() { return "\\/"; }

		void GenRandomBytes(void* destBuf, U32 bufSizeBytes);
		const char* FindFullProgPath();
		const char* GetProgDir();

		//make into a singleton?
		bool Startup();
		bool UpdateOS();
		void Shutdown();

		void SetInputManager(Handle inMgrHnd);// { inputMgrHnd = inMgrHnd; }
		//temporary to get info on reporting HID data
		void ListInputDevices() {}
		TypedHandle<InputDevInfo> FindInputDevList(U32* numDevs);
		void InitInput(WindowHnd targetWindow, TypedHandle<InputManager> inputMgr);

		//builds a window and attempts to attach the passed IGraphicsWrapper to the window; pass a null renderer to get a blank screen.
		//Returns true only if a window could be built and a IGraphicsWrapper attached.
		bool GetWindow(GfxWrapperHandle grpWrapper, WindowType type, U32 width, U32 height);
		HWND GetWindowHnd() const { return window; }
		IGraphicsWrapper* BuildGraphicsWrapper(RendererType type);
		void ShutdownGraphicsWrapper(GfxWrapperHandle grpWrapper);
		bool SetViewport(GfxWrapperHandle grpWrapper, WindowHnd vpHnd);
		void BeginRender(GfxWrapperHandle grpWrapper);
		void EndRender(GfxWrapperHandle grpWrapper);
		//audio ops
		IAudioWrapper* BuildAudioWrapper(AudioType type) { return NULL; }
		void ShutdownAudioWrapper(IAudioWrapper* audio) {}
		//handles messages from the message loop.
		//returns true if the program should quit!
		//must be static to properly attach to window class
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
		//void* OSVirAlloc(void* desiredAddr, size_t size);
		//void OSVirFree(void* ptr);
		//terminal ops
		void ClearTerm();
		//debug only calls
		void* GetOGLCtx() const;
		void* GetHDC() const;
	};
}
#endif //WIN32
#endif //WIN32PLATFORM_H