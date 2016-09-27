#include "StdAfx.h"
#include "Vector4.h"

using namespace LeEK;

//Class constants:
const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);

//Default constructor makes a zero vector.
Vector4::Vector4()
{
	x = y = z = w = 0.0f;
}

Vector4::Vector4(F32 vx, F32 vy, F32 vz, F32 vw)
{
	this->x = vx;
	this->y = vy;
	this->z = vz;
	this->w = vw;
}

Vector4::Vector4(const Vector3& v, F32 vw)
{
	x = v.X();
	y = v.Y();
	z = v.Z();
	this->w = vw;
}

String Vector4::ToString()
const {
	char rowBuf[196];
	
	sprintf_s(rowBuf, sizeof(rowBuf), "< %.4f, %.4f, %.4f, %.4f >", x, y, z, w);
	return rowBuf;
}
