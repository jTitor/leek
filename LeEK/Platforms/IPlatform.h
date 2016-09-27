#ifndef IPLATFORM_H
#define IPLATFORM_H
#pragma once
#include "PlatformCommon.h"
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "Audio/AudioManager.h"
#include "Memory/Handle.h"
#include "Input/Input.h"

namespace LeEK
{
	class IPlatform
	{
	protected:
		//if true, the mouse cursor will be locked to the center of the screen.
		bool shouldLockMouse;

		virtual U8 getUniversalKeyCode(U8 keyCode) = 0;
		virtual WindowHnd getWindow() = 0;
	public:
		enum WindowType { WINDOW, FULLSCREEN };
		IPlatform()
		{
			shouldLockMouse = false;
		}
		virtual ~IPlatform(void) {}
		bool ShouldLockMouse();
		void SetShouldLockMouse(bool val);
		virtual void GenRandomBytes(void* destBuf, U32 bufSizeBytes) = 0;
		virtual const char* FilesystemPathSeparator() = 0;
		virtual const char* FindFullProgPath() = 0;
		virtual const char* GetProgDir() = 0;
		virtual const PlatformType Type() = 0;
		virtual bool Startup() = 0;
		/**
		Handles messages from OS. Should return true if 
		OS is telling the program to quit, false otherwise
		*/
		virtual bool UpdateOS() = 0;	
		virtual void Shutdown() = 0;
		virtual void SetInputManager(Handle inMgrHnd) = 0;
		virtual void ListInputDevices() = 0;
		//also has counter to get the number of devices
		virtual TypedHandle<InputDevInfo> FindInputDevList(U32* numDevs) = 0;
		virtual void InitInput(WindowHnd targetWindow, TypedHandle<InputManager> inputMgr) = 0;
		inline void InitInput(TypedHandle<InputManager> inputMgr)
		{
			InitInput(getWindow(), inputMgr);
		}
		//graphics wrapper ops
		/**
		Builds the requested graphics wrapper (although it REALLY seems redundant,
		and this call may be removed in later versions). This is meant to only be called by
		static functions in IGraphicsWrapper.
		If NULL is returned, you can assume that no memory
		was allocated for the graphics wrapper.
		*/
		virtual IGraphicsWrapper* BuildGraphicsWrapper(RendererType type) = 0;
		virtual void ShutdownGraphicsWrapper(GfxWrapperHandle grpWrapper) = 0;
		virtual void BeginRender(GfxWrapperHandle grpWrapper) = 0;
		virtual void EndRender(GfxWrapperHandle grpWrapper) = 0;
		virtual bool GetWindow(GfxWrapperHandle grpWrapper, WindowType type, U32 width, U32 height) = 0;
		/**
		Sets the window that the graphics wrapper should draw to.
		Returns true if the viewport was successfully changed
		and can be drawn to, false otherwise.
		*/
		virtual bool SetViewport(GfxWrapperHandle grpWrapper, WindowHnd vpHnd) = 0;
		//audio ops
		virtual IAudioWrapper* BuildAudioWrapper(AudioType type) = 0;
		virtual void ShutdownAudioWrapper(IAudioWrapper* audio) = 0;
		//terminal ops
		virtual void ClearTerm() {};
	};
}
#endif //IPLATFORM_H