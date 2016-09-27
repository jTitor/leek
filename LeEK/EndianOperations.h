//include guard
#pragma once
#ifdef _MSC_VER
#include <intrin.h>
#endif
#include "Datatypes.h"

namespace EndianOperations
{
	//don't need to swap chars, since they're only a byte wide
	//we don't have datatypes larger than 64 bits, so there's nothing to handle
	//larger types

	//we only need to use alternative implementation if this isn't compiled in VS

	inline void swapByteOrder(U16& us)
	{
#ifdef _MSC_VER
		_byteswap_ushort(us);
#else
		us = (us >> 8) |
			 (us << 8);
#endif
	}

	inline void swapByteOrder(U32& ui)
	{
#ifdef _MSC_VER
		_byteswap_ulong(ui);
#else
		ui = (ui >> 24) |
			 ((ui<<8) & 0x00FF0000) |
			 ((ui>>8) & 0x0000FF00) |
			 (ui << 24);
#endif
	}

	inline void swapByteOrder(U64& ull)
	{
#ifdef _MSC_VER
		_byteswap_uint64(ull);
#else
		ull = (ull >> 56) |
			  ((ull<<40) & 0x00FF000000000000) |
			  ((ull<<24) & 0x0000FF0000000000) |
			  ((ull<<8) & 0x000000FF00000000) |
			  ((ull>>8) & 0x00000000FF000000) |
			  ((ull>>24) & 0x0000000000FF0000) |
			  ((ull>>40) & 0x000000000000FF00) |
			  (ull << 56);
#endif
	}
}