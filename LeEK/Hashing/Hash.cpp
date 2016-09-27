#include <StdAfx.h>
#include "Hash.h"
#include "Libraries/MurmurHash3/MurmurHash3.h"

using namespace LeEK;

//hide the seed value
namespace
{
	const U32 HASH_SEED = 0xA2490425;
}

//U32 LeEK::Hash(const String& key)
U32 LeEK::getHash(const void* val, U32 valLen)
{
	U32 result;
	//const char* keyData = key.c_str();
	MurmurHash3_x86_32(val, valLen, HASH_SEED, &result);
	return result;
}

//add overloads for basic types
