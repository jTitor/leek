//include guard
#pragma once

#include <cstdint>
//include file for Intel SSE intrinsics!
//#include <xmmintrin.h>
//#include <fpieee.h>
//#include <climits>

#ifndef CHAR_BIT
const int CHAR_BIT = 8;
#endif

//Definitions of atomic datatypes.

//We'll need to define what a float is
//not really a portable form for this, but floats are in practice always 32 bits
//and doubles 64 bit
static_assert(sizeof(float)*CHAR_BIT == 32, "float is not 32-bit on this architecture; please fix F32 typedef!");
typedef float F32;
static_assert(sizeof(double)*CHAR_BIT == 64, "double is not 64-bit on this architecture; please fix F64 typedef!");
typedef double F64;

//the 8-64 bit ints
typedef std::int8_t I8;
typedef std::uint8_t U8;
typedef std::int16_t I16;
typedef std::uint16_t U16;
typedef std::int32_t I32;
typedef std::uint32_t U32;
typedef std::int64_t I64;
typedef std::uint64_t U64;

//SIMD vector (4 32-bit floats)
//typedef __m128 VF32;

//NULL's not really a datatype, but we use it friggin everywhere.
#ifndef NULL
#define NULL nullptr
#endif

//For filesystem.
typedef U32 FileSz;