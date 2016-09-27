#pragma once
#include <Platforms/IPlatform.h>
#include <Platforms/Win32Platform.h>
#include <Platforms/LinuxPlatform.h>

namespace LeEK
{
	class Devices
	{
	private:
		static IPlatform* gPlat;
		static GfxWrapperHandle gGfx;
		static TypedHandle<InputManager> gInput;
	public:
		static IPlatform* GetPlatform();
		static TypedHandle<InputManager> GetInput();
		static GfxWrapperHandle GetGfx();
	};
}