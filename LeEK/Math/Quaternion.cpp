#include "StdAfx.h"
#include "Quaternion.h"
#include <sstream>

using namespace LeEK;

const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion(void)
{
}

//direct setter
Quaternion::Quaternion(F32 vx, F32 vy, F32 vz, F32 vw)
{
	x = vx;
	y = vy;
	z = vz;
	w = vw;

	Normalize();
}

//build from (unit?) axis and angle
Quaternion::Quaternion(const Vector3& axis, F32 angleRads)
{
	//we want to only use normalized quaternions;
	//we can't do that if the axis isn't a unit vector
	//normalize the axis if needed
	Vector3 finalAxis = axis.GetNormalized();
	//if(finalAxis.LengthSquared() != 1.0f)
	//{
	//	finalAxis = finalAxis.GetNormalized();
	//}

	//components based off of sine & cosine of the half angle
	F32 halfAngle = angleRads / 2.0f;
	F32 cos = Math::Cos(halfAngle);
	F32 sin = Math::Sin(halfAngle);

	//vector part = axis * sin(half angle).
	//scalar part = cos(half angle).
	x = sin * finalAxis.X();
	y = sin * finalAxis.Y();
	z = sin * finalAxis.Z();
	w = cos;
}

Matrix4x4 Quaternion::ToMatrix() const
{
	Quaternion adjustedRot = this->GetNormalized();
	//ensure q's normalized
	//if(adjustedRot.MagnitudeSquared() != 1.0f)
	//{
	//	adjustedRot = adjustedRot.GetNormalized();
	//}

	const F32 s = 2.0f; //would be 2.0f/MagSqr(q) if q wasn't normalized beforehand
	F32 xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
	xs = s*adjustedRot.X();		ys = s*adjustedRot.Y();		zs = s*adjustedRot.Z();
	wx = adjustedRot.W()*xs;	wy = adjustedRot.W()*ys;	wz = adjustedRot.W()*zs;
	xx = adjustedRot.X()*xs;	xy = adjustedRot.X()*ys;	xz = adjustedRot.X()*zs;
	yy = adjustedRot.Y()*ys;	yz = adjustedRot.Y()*zs;	zz = adjustedRot.Z()*zs;
	//wx = adjustedRot.W()*adjustedRot.X();	wy = adjustedRot.W()*adjustedRot.Y();	wz = adjustedRot.W()*adjustedRot.Z();
	//xx = adjustedRot.X()*adjustedRot.X();	xy = adjustedRot.X()*adjustedRot.Y();	xz = adjustedRot.X()*adjustedRot.Z();
	//yy = adjustedRot.Y()*adjustedRot.Y();	yz = adjustedRot.Y()*adjustedRot.Z();	zz = adjustedRot.Z()*adjustedRot.Z();

	return Matrix4x4(	1.0f - (yy + zz),	xy - wz,			xz + wy,			0.0f,	
						xy + wz,			1.0f - (xx + zz),	yz - wx,			0.0f,
						xz - wy,			yz + wx,			1.0f - (xx + yy),	0.0f,
						0.0f,				0.0f,				0.0f,				1.0f);
	//return Matrix4x4(	1.0f - s * (yy + zz),	s * (xy - wz),			s * (xz + wy),			0.0f,	
	//					s * (xy + wz),			1.0f - s * (xx + zz),	s * (yz - wx),			0.0f,
	//					s * (xz - wy),			s * (yz + wx),			1.0f - s * (xx + yy),	0.0f,
	//					0.0f,					0.0f,					0.0f,					1.0f);
}

//transpose this?
Quaternion Quaternion::FromMatrix(const Matrix4x4& M)
{
	//largely derived from http://www.flipcode.com/documents/matrfaq.html#Q52
	//calculate M's trace - sum of its diagonals. 
	//We'll only consider the rotation section of the matrix.
	F32 trace = M(0,0) + M(1,1) + M(2,2) + 1;
	F32 scale = 0.5f / Math::Sqrt(trace);
	//if the trace is positive, we have a very simple conversion
	if(trace > 0.0f)
	{
		//Vector3 axis((M(2,1) - M(1,2)) * scale, (M(0,2) - M(2,0)) * scale, (M(1,0) - M(0,1)) * scale);
		return Quaternion(	(M(2,1) - M(1,2)) * scale,
							(M(0,2) - M(2,0)) * scale,
							(M(1,0) - M(0,1)) * scale,
							0.25f / scale).GetNormalized();//axis, trace + 1.0f).GetNormalized();
	}

	//otherwise, we're in for more crap!
	//find the largest value along the diagonal
	U32 diagIndex = 0;
	F32 largestValue = M(diagIndex,diagIndex);
	//we only have to check the other two diagonals
	for(U32 i = 1; i < 3; i++)
	{
		if(M(i, i) > largestValue)
		{
			largestValue = M(i,i);
			diagIndex = i;
		}
	}

	switch(diagIndex)
	{
		//if M(0,0) was largest:
	case 0:
		scale = Math::Sqrt(M(0,0) - M(1,1) - M(2,2) + 1.0f) * 2.0f;
		return Quaternion(	0.5f / scale,//M(0,0) - M(1,1) - M(2,2) + 1.0f,
							(M(0,1) + M(1,0)) / scale,
							(M(0,2) + M(2,0)) / scale,
							(M(2,1) - M(1, 2)) / scale).GetNormalized();
		//if M(1,1) was largest:
	case 1:
		scale = Math::Sqrt(M(1,1) - M(0,0) - M(2,2) + 1.0f) * 2.0f;
		return Quaternion(	(M(0,1) + M(1,0)) / scale,
							0.5f / scale,//M(1,1) - M(0,0) - M(2,2) + 1.0f,
							(M(1,2) + M(2,1)) / scale,
							(M(0,2) - M(2, 0)) / scale).GetNormalized();
		//if M(2,2) was largest:
	case 2:
		scale = Math::Sqrt(M(2,2) - M(0,0) - M(1,1) + 1.0f) * 2.0f;
		return Quaternion(	(M(0,2) + M(2,0)) / scale,
							(M(2,1) + M(1,2)) / scale,
							0.5f / scale,//M(2,2) - M(0,0) - M(1,1) + 1.0f,
							(M(1,0) - M(0, 1)) / scale).GetNormalized();
	default:
		return Quaternion::Identity;
	}
}

Vector3 Quaternion::RotateVector(const Vector3& v)
const {

	//courtesy of Fabian Giesen, 
	//posted on http://mollyrocket.com/forums/viewtopic.php?t=833&sid=3a84e00a70ccb046cfc87ac39881a3d0
	Vector3 quatVec = Vector3(x, y, z);
	Vector3 t = 2.0f * Vector3::Cross(quatVec, v);
	return v + (w * t) + Vector3::Cross(quatVec, t);

	/*
	F32 vMult = 2.0f*(x*v.X() + y*v.Y() + z*v.Z());
	F32 crossMult = 2.0f*w;
	F32 pMult = crossMult*w - 1.0f;

	return Vector3(	pMult*v.X() + vMult*x + crossMult*(z*v.Y() - y*v.Z()),
					pMult*v.Y() + vMult*y + crossMult*(x*v.Z() - z*v.X()),
					pMult*v.Z() + vMult*z + crossMult*(y*v.X() - x*v.Y()));
	*/
}

Quaternion Quaternion::FromEulerAngles(F32 xRads, F32 yRads, F32 zRads)
{
	using namespace LeEK::Math;
	//build 3 basis quaternions, then multiply them together.
	//Quaternion x = Quaternion(Vector3(1, 0, 0), xRads)t;
	//Quaternion y = Quaternion(Vector3(0, 1, 0), yRads);
	//Quaternion z = Quaternion(Vector3(0, 0, 1), zRads);
	//return x * y * z;
	F32 cosX = Cos(xRads / 2.0f);
	F32 cosY = Cos(yRads / 2.0f);
	F32 cosZ = Cos(zRads / 2.0f);
	F32 sinX = Sin(xRads / 2.0f);
	F32 sinY = Sin(yRads / 2.0f);
	F32 sinZ = Sin(zRads / 2.0f);
	return Quaternion(	sinX*cosY*cosZ + cosX*sinY*sinZ,
						cosX*sinY*cosZ - sinX*cosY*sinZ,
						cosX*cosY*sinZ + sinX*sinY*cosZ,
						cosX*cosY*cosZ - sinX*sinY*sinZ);
}

Quaternion::~Quaternion(void)
{
}

String Quaternion::ToString() const 
{
	char rowBuf[128];
	
	sprintf_s(rowBuf, sizeof(rowBuf), "( %.4f, %.4f, %.4f, %.4f )", x, y, z, w);
	//std::stringstream sstrm;
	//sstrm << "( " << x << ", " << y << ", " << z << ", " << w << " )";
	//return sstrm.str();
	return rowBuf;
}
