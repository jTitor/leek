#pragma once
#include <Strings/String.h>

namespace LeEK
{
	namespace StringUtils
	{
		/**
		Attempts to match a wildcard against a given string.
		Returns true if the ENTIRETY of the string matches the wildcard,
		false otherwise.
		*/
		bool WildcardMatch(const char* str, const char* wildcard);
		//bool WildcardMatch(String str, char* wildcard) { return WildcardMatch(str.c_str(), wildcard); }
	}
}