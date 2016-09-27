#include "StdAfx.h"
#include "MathFunctions.h"

using namespace LeEK;

#ifndef EXTRA_PRECISION
	#define EXTRA_PRECISION
#endif

/*
const F32 Math::PI = 3.14159265f;
const F32 Math::TWO_PI = 2.0f * PI;
const F32 Math::PI_SQR = PI * PI;
const F32 Math::RAD_TO_DEG = 180.0f / PI;
const F32 Math::DEG_TO_RAD = PI / 180.0f;
*/

F32 Math::Sin(F32 thetaRads)
{
	using namespace LeEK::Math;
	//using Nick from devmaster.net's fast sine: http://devmaster.net/forums/topic/4648-fast-and-accurate-sinecosine/
	//first wrap the angle to the range [-pi, pi)
	F32 wrappedAngle = WrapPosNegPI(thetaRads);

	//then pass that into a parabola
	static const F32 B = 4.0f/PI;//1.27323954f; //4.0f/PI;
    static const F32 C = -4.0f/(PI * PI);//-0.405284735f; //-4.0f/(PI * PI);

    F32 y = (B + C * abs(wrappedAngle)) * wrappedAngle;

    #ifdef EXTRA_PRECISION
		//further refine the parabola!
        const F32 P = 0.225f;//0.218f;//0.225f;

        y = P * (y * abs(y) - y) + y;   // Q * y + P * y * abs(y)
		//y = Q * y + P * y * abs(y);
    #endif

	return y;
	//return sin(thetaRads);
}

F32 Math::ASin(F32 x)
{
	static const F32 scale = 0.391f;
	F32 x4 = x*x;
	x4 *= x4; //raise it to its proper power here
	return (1+scale*x4)*x;
}


/*
F32 Math::Cos(F32 thetaRads)
{
	//using Nick from devmaster.net's fast cosine: http://devmaster.net/forums/topic/4648-fast-and-accurate-sinecosine/
	//not quite the same as the sine - we have to first shift the angle by +pi/2, then by -2pi if theta was originally > pi/2
	F32 wrappedAngle = thetaRads - (PI/2.0f);//WrapPosNegPI(thetaRads);
	//huge hack!
	//FALSE = 0.0f
	//wrappedAngle -= (wrappedAngle > PI) && (2.0f * PI);

	const F32 B = 4.0f/PI;
    const F32 C = -4.0f/(TWO_PI);

    F32 y = -(B * wrappedAngle + C * wrappedAngle * abs(wrappedAngle));


    #ifdef EXTRA_PRECISION
    //  const F32 Q = 0.775;
        const F32 P = 0.225f;

        y = P * (y * abs(y) - y) + y;   // Q * y + P * y * abs(y)
    #endif

	return y;
}*/
