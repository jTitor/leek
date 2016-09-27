#pragma once
#include "Datatypes.h"

namespace LeEK
{
	class Duration
	{
	private:
		F64 spanLength; //duration in ms
	public:
		Duration(F64 ticks) 
		{
#ifdef WIN32
			spanLength = ticks;
#else
			spanLength = ticks; //fix this up - will this be true on other platforms?
#endif
		}
		//~Duration(void);
		inline U64 ToNanoseconds() { return (U64)(spanLength * 1000000.0); }
		inline F64 ToMicroseconds() { return spanLength * 1000.0; }
		inline F32 ToMilliseconds() { return (F32)spanLength; }
		inline F32 ToSeconds() { return (F32)(spanLength / 1000.0); }
		inline F32 ToMinutes() { return ToSeconds() / 60.0f; }
		inline F32 ToHours() { return ToMinutes() / 60.0f; }
	};
}
