#pragma once
#include "Memory/STLAllocHook.h"
#include "Math/MathFunctions.h"
#include <string>
#include <algorithm>
//for now, Strings are a typedef
namespace LeEK
{
	//typedef std::string String;
	typedef char charType;
	typedef LeEK::STLStrAllocHook<charType> stlHook;
	typedef std::basic_string<charType, std::char_traits<charType>, stlHook> String;
	//must now implement casts and concats

	String operator+ (const String& lhs, const std::string& rhs);
	String operator+ (const std::string& lhs, const String& rhs);
	String operator+ (const String& lhs, const char* rhs);
	String operator+ (const char* lhs, const String& rhs);
	
	template <class valType>
	String StrFromVal(valType val)
	{
		//should change this, may make untracked allocs
		return String(std::to_string(val).c_str());
	}

	//internal call for the HexStrFromVal functions.
	//note that this does a heap alloc - 
	//the functions need to delete this data after use!
	template <class valType>
	String hexWithPadding(valType val, size_t padLen)
	{
		if(val == 0)
		{
			return String("0x0");
		}
		char fmtStr[16];
		//a little weird - '%%' in a format string prints '%'
		//needed to specify the level of padding desired
		//also need to exclude the '0x' at the end of the string
		sprintf_s(fmtStr, 16, "%%#0%dx", padLen + 2);
		char resStr[32] = {0};//= CustomArrayNew<char>(32, STRING_ALLOC, "HexBufAlloc");
		sprintf_s(resStr, 32, fmtStr, val);
		return String(resStr);
	}

	template<typename T>
	String HexStrFromVal(T val, size_t padding = 0)
	{
		return hexWithPadding(val, Math::Max(sizeof(T)*2, padding));
	}
	//alternate - hex conversion
	/*String HexStrFromVal(I16 val);
	String HexStrFromVal(I32 val);
	String HexStrFromVal(F32 val);
	String HexStrFromVal(I64 val);
	String HexStrFromVal(F64 val);*/
	String StrFromPtr(void* ptr);

	//screw the templates, just do the native concats by hand
	//ok, might be harder than expected (U32 aliases with F32) but still
	String operator+ (const String& lhs, const I32 rhs); //{ return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const I32 lhs, const String& rhs); //{ return operator+(StrFromVal(lhs), rhs); }
	String operator+ (const String& lhs, const U32 rhs);
	String operator+ (const U32 lhs, const String& rhs);
	String operator+ (const String& lhs, const U64 rhs);
	String operator+ (const U64 lhs, const String& rhs);
	String operator+ (const String& lhs, const F32 rhs);
	String operator+ (const F32 lhs, const String& rhs);
	String operator+ (const String& lhs, const F64 rhs);
	String operator+ (const F64 lhs, const String& rhs);

	String ToLower(const String& str);
}
