//include guard
#pragma once

#include "Platforms/Platforms.h"
#include "Datatypes.h"
#include <limits>
#include <cmath>
#include <cfloat>
#include <xmmintrin.h>

namespace LeEK
{
	namespace Math
	{
		#pragma region Constants
		const F32 PI = 3.14159265f;//3.1415927f;
		const F32 TWO_PI = 6.28318531f;//2.0f * PI;
		const F32 PI_OVER_2 = 0.5f * PI;//1.57079632f;
		const F32 PI_OVER_4 = 0.25f * PI;//0.78539816f;
		const F32 PI_SQR = PI * PI;//9.86960440f;
		const F32 RAD_TO_DEG = 180.0f / PI;
		const F32 DEG_TO_RAD = PI / 180.0f;
		#pragma endregion

		/**
		Finds the sign of a signed number.
		@return 1 if the number is positive, 0 if the number is 0, and -1 if the number is negative.
		*/
		template<typename T>
		T Sign(T val)
		{
			if(val > 0)
			{
				return 1;
			}
			else if(val < 0)
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}

		/**
		Determines if two numbers are approximately equal.
		Mainly used to determine if a very small floating-point number is near zero,
		because of the structure of IEEE-754 floats.
		*/
        template <typename T>
        bool ApproxEqual(T a, T b)
        {

			//Testing difference against absolute epsilon can't handle error between very small values;
			//see http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
			//Instead, combine a quick epsilon test for differences very close to zero,
			//and a int reinterpretation (see http://randomascii.wordpress.com/2012/01/23/stupid-float-tricks-2/)
			//to find significant differences in all other situations.
			//Note that this relies on properties of IEEE floats.
        	float aVal = a;
        	float bVal = b;
			float absDiff = fabs(aVal - bVal);
			if(absDiff <= FLT_EPSILON)
			{
				return true;
			}
			//If signs are different and the difference is past epsilon,
			//we can be pretty sure this isn't -0 and +0, so assume they're different.
			//Int reintepretation won't work when signs differ, as the sign bit is the MOST significant bit
			//and so the two values will appear to be billions of units apart.
			if(Sign(aVal) != Sign(bVal))
			{
				return false;
			}
			//Otherwise, there's a fairly large difference between floats.
			//Reinterpret the floats as ints and get the integer difference to find how large that difference is
			//Specifically, the absolute value of the integer difference = 
			//1 + (number of representable floats between values)
			//since IEEE floats have mantissa as least significant bits,
			//and the exponent is the next significant bits past the mantissa
			int diff = *(reinterpret_cast<int*>(&aVal)) - *(reinterpret_cast<int*>(&bVal));
			//the numbers are approximately equal if diff is less than the maximum tolerance value
			const int MAX_DIFF = 1;
            return abs(diff) < MAX_DIFF;
        }

		#pragma region Trigonometrics
		//fast sine and cosine
		extern F32 Sin(F32 thetaRads);
		inline F32 Cos(F32 thetaRads)
		{ 
			F32 x = thetaRads + (PI_OVER_2);
			//kludgy as hell!
			//if true = 1 * 2pi
			//else = 0
			//x -= ((float)(x > PI)) * TWO_PI;
			return Sin(x); 
		}
		inline F32 Tan(F32 thetaRads)	{ return Sin(thetaRads) / Cos(thetaRads); }

		extern F32 ASin(F32 x);
		inline F32 ACos(F32 x) { return (PI_OVER_2) - ASin(x); }
		#pragma endregion

		//angle wrapping functions copied verbatim from stackoverflow by "Lior Kogan":
		// modulus - similar to matlab's mod()
		// result is always positive. not similar to fmod()
		// Mod(-3,4)= 1   fmod(-3,4)= -3
		F32 inline __fastcall Mod(F32 x, F32 y)
		{
			if (ApproxEqual(y, 0.0f))
			{
				return x;
			}

			return x - y * floor(x/y);
		}

		#pragma region Angle Wrapping
		// wrap [rad] angle to [-PI..PI)
		inline F32 WrapPosNegPI(F32 fAng)
		{
			return Mod(fAng + PI, TWO_PI) - PI;//Mod(fAng + PI, TWO_PI) - PI;
		}

		// wrap [rad] angle to [0..TWO_PI)
		inline F32 WrapTwoPI(F32 fAng)
		{
			return Mod(fAng, TWO_PI);
		}

		// wrap [deg] angle to [-180..180)
		inline F32 WrapPosNeg180(F32 fAng)
		{
			return Mod(fAng + 180.0f, 360.0f) - 180.0f;
		}

		// wrap [deg] angle to [0..360)
		inline F32 Wrap360(F32 fAng)
		{
			return Mod(fAng ,360.0f);
		}
		#pragma endregion

		//alternate version of this is the SSE:
		//actually is faster, too. weird that it seems to be less accurate, though
		inline F32 SseSqrt(F32 n)
		{
			//for some weird reason, _mm_set_ps sets vector in REVERSE order.
			//then again, that's not important here, either
			//since we can use _ss intrinsics to operate on one component only
			__m128 vec = _mm_set_ss(n);
			//get reciprocal of the square root
			__m128 res = _mm_rsqrt_ss(vec);
			//and multiply by the original values to get the result
			__m128 ret = _mm_mul_ss(vec, res);
			F32 result;
			_mm_store_ss(&result, ret);
			return result;
		}

		//TODO: SSE vectorized sqrt

#ifdef WIN32
		/*
		//fast squareroot from http://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
		//note that this is for x86 processors only!
		//the calling conventions also restrict this version to the VC++ compiler.
		inline F64 __declspec(naked) __fastcall Sqrt(F64 n)
		{
			_asm fld qword ptr [esp+4]	//load the parameter onto the stack
			_asm fsqrt					//take square root of the value on the top of the stack
			_asm ret 8					//return the top 8 bytes of the stack (our double)
		}
		*/
#define Sqrt(x) SseSqrt(x)
#else
		//very bad!!!
		//replace with gcc assembly equivalent
    #define Sqrt(x) sqrt(x)
#endif

		template<typename T>
		inline T Max(T x, T y)
		{
			return (x >= y ? x : y);
		}

		template<typename T>
		inline T Min(T x, T y)
		{
			return (x <= y ? x : y);
		}

		template <typename T>
		inline T Clamp(T val, T min, T max)
		{
			if(val > max)
			{
				return max;
			}
			else if(val < min)
			{
				return min;
			}
			else
			{
				return val;
			}
		}

		//binary functions
		template<class T> inline T RoundDown(T val, size_t alignment) {return val & -(int)alignment;}
		template<class T> inline T RoundUp(T val, size_t alignment) {return (val + (alignment-1)) & -(int)alignment;}
		template<class T> inline T* AlignDown(T* pointer, size_t alignment) {return (T*)((size_t)pointer & -(int)alignment);}
		template<class T> inline T* AlignUp(T* pointer, size_t alignment) {return (T*)(((size_t)pointer + (alignment-1)) & -(int)alignment);}

		//this isn't templated to avoid accidental usage w/ signed numbers.
		inline bool IsPowerOfTwo(U64 val)
		{
			//if a number is a power of two,
			//then the number should be 0 when AND'd w/
			//itself minus 1 (all powers of two have only one 1 bit,
			//and the number minus 1 won't have a 1 bit in that position).
			return ((val != 0) && !(val & (val-1)));
		}
		inline bool IsPowerOfTwo(U32 val)
		{
			//if a number is a power of two,
			//then the number should be 0 when AND'd w/
			//itself minus 1 (all powers of two have only one 1 bit,
			//and the number minus 1 won't have a 1 bit in that position).
			return ((val != 0) && !(val & (val-1)));
		}

		/**
		Returns the lowest value of two larger than or equal to the given value.
		*/
		inline U64 NearestPowOf2(U64 val)
		{
			U64 shiftVal = val;
			//a little goofy, but we can get the closest power of two this way
			//sub by 1 to enable bits below the last trailing 1
			shiftVal--;
			//and OR by the new value shifted by a power of two
			//to enable the bits below the last trailing 1
			shiftVal |= shiftVal >> 1;
			shiftVal |= shiftVal >> 2;
			shiftVal |= shiftVal >> 4;
			shiftVal |= shiftVal >> 8;
			shiftVal |= shiftVal >> 16;
			shiftVal |= shiftVal >> 32;
			//keep doing this, and every bit below the closest pow2 should be enabled
			//now add 1 to get the actual power
			shiftVal++;
			return shiftVal;
		}
		/**
		Returns the lowest value of two larger than or equal to the given value.
		*/
		inline U32 NearestPowOf2(U32 val)
		{
			U32 shiftVal = val;
			shiftVal--;
			shiftVal |= shiftVal >> 1;
			shiftVal |= shiftVal >> 2;
			shiftVal |= shiftVal >> 4;
			shiftVal |= shiftVal >> 8;
			shiftVal |= shiftVal >> 16;
			shiftVal++;
			return shiftVal;
		}
		/**
		Returns the lowest value of two larger than or equal to the given value.
		*/
		inline U16 NearestPowOf2(U16 val)
		{
			U16 shiftVal = val;
			shiftVal--;
			shiftVal |= shiftVal >> 1;
			shiftVal |= shiftVal >> 2;
			shiftVal |= shiftVal >> 4;
			shiftVal |= shiftVal >> 8;
			shiftVal++;
			return shiftVal;
		}

		/**
		Gets the absolute value of a numeric value.
		*/
		template<typename T>
		T Abs(T val)
		{
			return std::abs(val);
		}

		/**
		Gets the base-10 logarithm of a given value.
		*/
		template<typename T>
		T Log10(T val)
		{
			return std::log10(val);
		}

		/**
		Gets the <b>power</b>'th power of a given value.
		*/
		template<typename T>
		T Pow(T base, int power)
		{
			return std::pow(base, power);
		}
	}
}
