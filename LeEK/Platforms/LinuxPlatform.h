#ifdef LINUXPLATFORM_H
#define LINUXPLATFORM_H
#pragma once
#ifdef __linux__
#include "IPlatform.h"
//window manager headers
#include<X11/X.h>
#include<X11/Xlib.h>

namespace LeEK
{
	class LinuxPlatform : public IPlatform
	{
	private:
		//Fields managing the game window.
		Display *disp;
		Window  win;
		Colormap cMap;

		bool postVisualSetup();
	protected:
		U8 getUniversalKeyCode(U8 keyCode);
	public:
		void GenRandomBytes(void* destBuf, U32 bufSizeBytes);
		const char* FilesystemPathSeparator();
		const char* FindFullProgPath();
		const char* GetProgDir();
		const PlatformType Type();
		//template<class T : IPlatform> static void SetPlatform(T platform) {  }
		//static void InitInstance(IPlatform* plat);
		bool Startup();
		bool UpdateOS();	//handles messages from OS. Should return true if OS is telling the program to quit, false otherwise
		void Shutdown();
		void SetInputManager(Handle inMgrHnd);
		void ListInputDevices();
		//also has counter to get the number of devices
		TypedHandle<InputDevInfo> FindInputDevList(U32* numDevs);
		void BuildHIDDetailList();
		void RegisterAllInputDevices();
		//graphics wrapper ops
		GfxWrapperHandle BuildGraphicsWrapper(RendererType type);
		void ShutdownGraphicsWrapper(GfxWrapperHandle grpWrapper);
		void BeginRender(GfxWrapperHandle grpWrapper);
		void EndRender(GfxWrapperHandle grpWrapper);
		bool GetWindow(GfxWrapperHandle grpWrapper, WindowType type, U32 width, U32 height);
		//audio ops
		IAudioWrapper* BuildAudioWrapper(AudioType type);
		void ShutdownAudioWrapper(IAudioWrapper* audio);
		//terminal ops
		void ClearTerm() {};
	};
}
#endif //__linux__
#endif //LINUXPLATFORM_H