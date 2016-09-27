#include "Random.h"
#include "Strings/String.h"
#include "Logging/Log.h"

using namespace LeEK;

//uses implementation of WELL Algorithm
//at http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf

/* initialize state to random bits */
static U32 state[16] = {	0xDECAFADE, 0x267aa643, 0x59512a15, 0x46a3f7d0, 
							0x508af194, 0x4969d7e4, 0x2fa7f841, 0x7fa1c079, 
							0x84587896, 0xe7679d54, 0xeaa7d6cc, 0xbedfdb56, 
							0x01aac3de, 0x3cd71b53, 0x681ae5ba, 0xc9e8fa19};

/* init should also reset this to 0 */
static U32 indexr = 0;
static const U32 RNG_MAX = INT_MAX;

void ResetToSeed(U32* seedValBuf, U32 numSeedElem)
{
	memcpy(state, seedValBuf, numSeedElem*sizeof(U32));
	indexr = 0;
}

/* return 32 bit random number */
U32 WELLRNG512(void)
{
	U32 a, b, c, d;
	a = state[indexr];
	c = state[(indexr+13)&15];
	b = a^c^(a<<16)^(c<<15);
	c = state[(indexr+9)&15];
	c ^= (c>>11);
	a = state[indexr] = b^c; 
	d = a^((a<<5)&0xDA442D24UL);
	indexr = (indexr + 15)&15;
	a = state[indexr];
	state[indexr] = a^b^d^(a<<2)^(b<<18)^(c<<28);
	return state[indexr];
}

I32 Random::InRange(I32 min, I32 max)
{
	//get a number
	U32 initVal = WELLRNG512();
	//must be within [0, MAX)
	if (initVal == RNG_MAX)
	{
		//if it's not, get another.
		return InRange(min,max);
	}
	//now it should be in in [0, MAX)
	//start converting to range
	I32 range = max-min;
	U32 remainder = RNG_MAX % range;
	U32 bucket = RNG_MAX / range;
	//the obtained number is out of range
	//if it's past RNG_MAX-remainder -
	//the number can't be evenly divided by the bucket number,
	//so it's not going to be equally likely to other numbers in the range.
	if(initVal < RNG_MAX - remainder)
	{
		//if in the range, you can get a number in the range
		//by normalizing by the bucket number
		//the value would be within [0, range),
		//so we can safely cast to I32
		return min + ((I32)(initVal / bucket));
	}
	return InRange(min,max);
}

//returns a random val in [0,1]
F32 getNormalizedFloat()
{
	return ((F32)Random::InRange(0,RNG_MAX))/RNG_MAX;
}

//returns a random val in [min, max]
//note this is a fully closed range!
F32 Random::InRange(F32 min, F32 max)
{
	//first, get a random val in [0,1]
	F32 initVal = getNormalizedFloat();
	F32 range = max-min;
	return min + initVal*range;
}

//returns a random vector bounded by the box
//formed by min and max.
//note this is a fully closed range!
Vector3 Random::InSpace(Vector3 min, Vector3 max)
{
	//Ensure vectors actually are min and max.
	Vector3::MakeBoxCorners(&min, &max);
	return Vector3(	InRange(min.X(), max.X()),
					InRange(min.Y(), max.Y()),
					InRange(min.Z(), max.Z()));
}

//returns a quaternion that has been randomly rotated along these ranges
Quaternion Random::InEulerRange(Vector3 min, Vector3 max)
{
	Vector3::MakeBoxCorners(&min, &max);
	return Quaternion::FromEulerAngles(	InRange(min.X(), max.X()),
										InRange(min.Y(), max.Y()),
										InRange(min.Z(), max.Z()));
}

Vector3 Random::InCube(F32 cubeMin, F32 cubeMax)
{
	return InSpace(Vector3::One * cubeMin, Vector3::One * cubeMax);
}

Vector3 Random::InCube(F32 cubeSideLen)
{
	cubeSideLen = Math::Abs(cubeSideLen);
	F32 halfLen = cubeSideLen / 2.0f;
	return InCube(-halfLen, halfLen);
}

Quaternion Random::InCubicEulerRange(F32 minRad, F32 maxRad)
{
	return InEulerRange(Vector3::One * minRad, Vector3::One * maxRad);
}
