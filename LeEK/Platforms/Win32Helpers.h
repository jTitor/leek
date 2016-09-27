#pragma once
#ifdef WIN32
#include "Strings/String.h"
#include <wtypes.h>

namespace LeEK
{
	namespace Win32Helpers
	{
		void LogError(LONG errCode);
		inline void LogLastError() { LogError(GetLastError()); }

		//String conversion functions.
		LPWSTR MultiToWide(const char* str, long len);
		String WideToMulti(const wchar_t* str, long len);
		String BSTRToMulti(BSTR str);
		BSTR MultiToBSTR(const String& str);
		BSTR MultiToBSTR(const std::string& str);
		BSTR MultiToBSTR(const char* str, size_t len);
	}
}

#endif //WIN32
