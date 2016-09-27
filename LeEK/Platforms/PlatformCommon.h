#pragma once
#include "Datatypes.h"

namespace LeEK
{
	class InputDevInfo
	{
	public:
		enum DevType { HID, Mouse, Keyboard, TypeSize } Type;
		struct AxisInfo
		{
			F32 Midpoint;
			F32 HalfRange;
		} Axii[16];
		//the number of buttons and axii is static, but keep the info anyway.
		U8 NumBtns, NumAxis;
		//a platform-specific identifier.
		//Used in the InputManager's ID resolver map.
		size_t PlatHandle;
		InputDevInfo()
		{
			Type = TypeSize;
			NumBtns = 0;
			NumAxis = 0;
			PlatHandle = 0;
		}
	};

	typedef void* WindowHnd;

	enum PlatformType { Win32, GNU, OSX, OS_INVALID };
}