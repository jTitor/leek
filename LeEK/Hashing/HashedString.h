#pragma once
#include <Strings/String.h>
#include <Datatypes.h>

namespace LeEK
{
	class HashedString
	{
	private:
		U32 value;
		String plainText;
	public:
		HashedString(const String& text);
		~HashedString(void);
		inline const String& OriginalString() const { return plainText; }
		inline const U32 Value() const { return value; }
	};
}
