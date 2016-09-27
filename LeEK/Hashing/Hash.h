//Uses MurmurHash3 from http://code.google.com/p/smhasher/wiki/MurmurHash3
//NOT meant for cryptography, Hash* functions all use the same seed
#pragma once
#include "Datatypes.h"
#include "Strings/String.h"

namespace LeEK
{
	//note that this only takes chunks of data, not straight values.
	U32 getHash(const void* val, U32 valLen);

	static U32 Hash(const char* key, size_t len = 0) 
	{
		if(len != 0)
		{
			return getHash(key, len);
		}
		return getHash(key, strlen(key));
	}

	template<typename T>
	static U32 Hash(const T& key) { return getHash(&key, sizeof(T)); }

	static U32 Hash(const String& key) { return Hash(key.c_str(), key.length()); }

	static U32 Hash(void* key) { return getHash(key, sizeof(void*)); }

	static U32 Hash(const char* key) { return Hash(key, strlen(key)); }
}
