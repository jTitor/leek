#include "StdAfx.h"
#include "Vector3.h"

using namespace LeEK;

//Class constants:
const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Forward(0.0f, 0.0f, -1.0f); //right-handed coordinate system


//Default constructor makes a zero vector.
Vector3::Vector3()
{
	tuple[0] = tuple[1] = tuple[2] = 0.0f;
}

Vector3::Vector3(F32 vx, F32 vy, F32 vz)
{
	tuple[0] = vx;
	tuple[1] = vy;
	tuple[2] = vz;
}

String Vector3::ToString()
const {
	char rowBuf[128];
	sprintf_s(rowBuf, sizeof(rowBuf), "< %.4f, %.4f, %.4f >", tuple[0], tuple[1], tuple[2]);
	return rowBuf;
}

const F32* Vector3::ToFloatArray() const
{
	return tuple;
}