#pragma once
#ifdef __linux__
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#define mh3_uint32_t uint32_t


#define strcpy_s(x,y,z) strncpy( x,z,y )
#define strcat_s(x,y,z) strncat( x,z,y )
#define memcpy_s(x,y,z,w) memcpy(x,z,w)
#define INT_MAX WINT_MAX
#define sprintf_s snprintf

#define i64 int64_t
#endif
