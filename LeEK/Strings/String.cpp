#include "String.h"
#include "Math/MathFunctions.h"

using namespace LeEK;

static const int STRING_ALLOC = 10;
namespace LeEK
{
	//this is a huge pain!!!

	String operator+ (const String& lhs, const std::string& rhs)
	{ 
		String result;
		result.reserve(lhs.size() + rhs.size());
		result += lhs;
		result += rhs.c_str();
		return result;
	}
	String operator+ (const std::string& lhs, const String& rhs)
	{
		String result;
		result.reserve(lhs.size() + rhs.size());
		result += lhs.c_str();
		result += rhs;
		return result;
	}
	String operator+ (const String& lhs, const char* rhs)
	{
		String result;
		result.reserve(lhs.size() + strlen(rhs));
		result += lhs;
		result += rhs;
		return result;
	}

	String operator+ (const char* lhs, const String& rhs)
	{
		String result;
		result.reserve(strlen(lhs) + rhs.size());
		result += lhs;
		result += rhs;
		return result;
	}

	/*
	String HexStrFromVal(I16 val)
	{
		return hexWithPadding(val, 4);
	}
	String HexStrFromVal(I32 val)
	{
		return hexWithPadding(val, 8);
	}
	String HexStrFromVal(F32 val)
	{
		return hexWithPadding(val, 8);
	}
	String HexStrFromVal(I64 val)
	{
		return hexWithPadding(val, 16);
	}
	String HexStrFromVal(F64 val)
	{
		return hexWithPadding(val, 16);
	}
	String HexStrFromVal(I64 val, U16 padding)
	{
		return hexWithPadding(val, Math::Max((U16)0,padding));
	}
	String HexStrFromVal(F64 val, U16 padding)
	{
		return hexWithPadding(val, Math::Max((U16)0,padding));
	}*/

	String StrFromPtr(void* ptr)
	{
		char resStr[32] = {0};
		sprintf_s(resStr, 32, "%p", ptr);
		return String(resStr);
	}

	String operator+ (const String& lhs, const I32 rhs) { return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const I32 lhs, const String& rhs) { return operator+(StrFromVal(lhs), rhs); }
	String operator+ (const String& lhs, const U32 rhs) { return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const U32 lhs, const String& rhs) { return operator+(StrFromVal(lhs), rhs); }
	String operator+ (const String& lhs, const U64 rhs) { return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const U64 lhs, const String& rhs) { return operator+(StrFromVal(lhs), rhs); }
	String operator+ (const String& lhs, const F32 rhs) { return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const F32 lhs, const String& rhs) { return operator+(StrFromVal(lhs), rhs); }
	String operator+ (const String& lhs, const F64 rhs) { return operator+(lhs, StrFromVal(rhs)); }
	String operator+ (const F64 lhs, const String& rhs) { return operator+(StrFromVal(lhs), rhs); }

	String ToLower(const String& str)
	{
		String res = str.c_str();
		std::transform(str.begin(), str.end(), res.begin(), ::tolower);
		return res;
	}
}
