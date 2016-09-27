#ifdef WIN32
#include "Win32Helpers.h"
#include "Logging/Log.h"
#include "Math/MathFunctions.h"
#include <iostream>
#include <Windows.h>

using namespace LeEK;

const U32 MAX_ERR_LEN = 255;

char errBuf[MAX_ERR_LEN+1];

void Win32Helpers::LogError(LONG errCode)
{
	//specify the error
	LPTSTR errStr = L"";
	//Formatting from StackOverflow, Q#455434
	FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM 	// use system message tables to retrieve error text
					|FORMAT_MESSAGE_ALLOCATE_BUFFER // allocate buffer on local heap for error text
					|FORMAT_MESSAGE_IGNORE_INSERTS,  // Important! will fail otherwise, since we will not (and CANNOT) pass insertion parameters
					NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
					errCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&errStr,  // output 
					0, // minimum size for output buffer
					NULL);   // arguments - see note 
	if(errStr == NULL)
	{
		return;
	}
	//std::wcout << L"Error: " << errStr << "\n";
	size_t errLen = Math::Min((U32)wcstombs(NULL, errStr, 0), MAX_ERR_LEN);
	wcstombs(errBuf, errStr, errLen + 1);
	/*
	Log::E("Error: ");
	Log::E(errBuf);
	Log::E("\n");
	*/
	LogE(String("Error: ") + errBuf);
}

LPWSTR Win32Helpers::MultiToWide(const char* str, long len)
{
	LPWSTR res = NULL;

	//Get the length this string would be as a multi-char string.
	int wideLen = MultiByteToWideChar(	CP_ACP, //use ANSI code page
										0,		//no special options
										str,
										len,
										NULL,	//not actually converting, just want length
										0);

	if(!wideLen)
	{
		LogLastError();
		return NULL;
	}

	//make the result string, fill with nulls
	res = (LPWSTR)LMalloc(sizeof(WCHAR)*wideLen, AllocType::PLAT_ALLOC, "PlatStrAlloc");
	memset((void*)res, 0, sizeof(WCHAR)*wideLen);
	//now actually convert!
	wideLen = MultiByteToWideChar(	CP_ACP, //use ANSI code page
									0,		//no special options
									str,
									len,
									res,
									wideLen);

	//now string should be converted
	return res;
}

String Win32Helpers::WideToMulti(const wchar_t* str, long len)
{
	//Get the length this string would be as a multi-char string.
	int multiLen = WideCharToMultiByte(	CP_ACP, //use ANSI code page
										0,		//no special options
										str,
										len,
										NULL,	//not actually converting, just want length
										0,
										NULL,
										NULL);
	//make the result string, fill with nulls
	String result(len, '\0');
	//now actually convert!
	multiLen = WideCharToMultiByte(	CP_ACP, //use ANSI code page
									0,		//no special options
									str,
									len,
									&result[0],	//weird; you are now directly accessing the string's buffer
									multiLen,
									NULL,
									NULL);

	//now string should be converted
	return result;
}

String Win32Helpers::BSTRToMulti(BSTR str)
{
	//BSTRs are directly readable as wchar*.
	int wideLen = SysStringLen(str);
	return WideToMulti((wchar_t*)str, wideLen);
}

BSTR Win32Helpers::MultiToBSTR(const char* str, size_t len)
{
	//Get the length this string would be as a wide string.
	int wideLen = MultiByteToWideChar(	CP_ACP,	//use ANSI code page
										0,		//no special options
										str,
										len,
										NULL,	//not actually converting, just want length
										0);

	BSTR result = SysAllocStringLen(NULL, wideLen);

	wideLen = MultiByteToWideChar(	CP_ACP,	//use ANSI code page
										0,		//no special options
										str,
										len,
										result,
										wideLen);

	return result;
}

BSTR Win32Helpers::MultiToBSTR(const String& str)
{
	return MultiToBSTR(str.c_str(), str.length());
}

BSTR Win32Helpers::MultiToBSTR(const std::string& str)
{
	return MultiToBSTR(str.c_str(), str.length());
}
#endif //WIN32