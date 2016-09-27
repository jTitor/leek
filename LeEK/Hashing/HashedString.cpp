#include "StdAfx.h"
#include "HashedString.h"
#include "Hashing/Hash.h"

using namespace LeEK;

HashedString::HashedString(const String& text)
{
	value = Hash(text);
	plainText = text;
}


HashedString::~HashedString(void)
{
}
