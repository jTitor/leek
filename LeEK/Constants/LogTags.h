#pragma once
#include "Datatypes.h"
#include "Strings/String.h"

namespace LeEK
{
	typedef U64 LogChnl_t;
	namespace LogTags
	{
		//remember that these constants need static linkage!
		static const char* MEM_ALLOC = "MemAlloc";
		static const char* GEOM_INIT = "GeomInit";
		static const char* FILESYS_ALLOC = "FileSysAlloc";

		static const LogChnl_t MEM_CHNL = 1;
		static const LogChnl_t RES_CHNL = 1 << 1;
		static const LogChnl_t ALL_CHNL = ~0;
	}
}
