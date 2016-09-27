#include "StdAfx.h"
#include "MathFunctions.h"
#include "Matrix3x3.h"
#include <sstream>

using namespace LeEK;

const Matrix3x3 Matrix3x3::Zero(	0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f);
const Matrix3x3 Matrix3x3::Identity(	1.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f,
										0.0f, 0.0f, 1.0f);

Matrix3x3::Matrix3x3()
{
	Matrix3x3 (	0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f);
}

Matrix3x3::Matrix3x3(	F32 v11, F32 v12, F32 v13, 
						F32 v21, F32 v22, F32 v23,
						F32 v31, F32 v32, F32 v33)
{
	(*this)(0,0) = v11;
	(*this)(0,1) = v12;
	(*this)(0,2) = v13;

	(*this)(1,0) = v21;
	(*this)(1,1) = v22;
	(*this)(1,2) = v23;

	(*this)(2,0) = v31;
	(*this)(2,1) = v32;
	(*this)(2,2) = v33;
}

Matrix3x3::Matrix3x3(	const Vector3& row1,
						const Vector3& row2,
						const Vector3& row3)
{
	Matrix3x3(	row1.X(),	row1.Y(),	row1.Z(),
				row2.X(),	row2.Y(),	row2.Z(),
				row3.X(),	row3.Y(),	row3.Z());
}

String Matrix3x3::ToString()
{
	//std::stringstream sstrm; //concatenate the string from here
	//sstrm << "\n";
	String resStr = "\n";
	char rowBuf[128];
	for(U32 i = 0; i < MATRIX_SIZE; i++)
	{
		//print a row of the matrix
		sprintf_s(rowBuf, sizeof(rowBuf), "\t[ %.4f, %.4f, %.4f ]\n", (*this)(i, 0), (*this)(i, 1), (*this)(i, 2));
		//sstrm << "\t[ " << (*this)(i, 0) << ", " << (*this)(i, 1) << ", " << (*this)(i, 2) << " ]" << "\n";
		resStr += rowBuf;
	}
	return resStr;//sstrm.str();
}

Vector3 Matrix3x3::Multiply(const Vector3& v)
{
	return Vector3(	v.X() * (*this)(0, 0) + v.Y() * (*this)(1, 0) + v.Z() * (*this)(2, 0),
					v.X() * (*this)(0, 1) + v.Y() * (*this)(1, 1) + v.Z() * (*this)(2, 1),
					v.X() * (*this)(0, 2) + v.Y() * (*this)(1, 2) + v.Z() * (*this)(2, 2));
}

Matrix3x3 Matrix3x3::BuildTranslation(F32 x, F32 y)
{
	Matrix3x3 result = Matrix3x3(	1.0f,	0.0f,	0.0f,
									0.0f,	1.0f,	0.0f,
									x,		y,		1.0f);
	return result;
}
Matrix3x3 Matrix3x3::BuildRotation2D(F32 angleRads)
{
	Matrix3x3 result = Matrix3x3(	Math::Cos(angleRads),	Math::Sin(angleRads),	0.0f,
									-Math::Sin(angleRads),	Math::Cos(angleRads),	0.0f,
									0.0f,					0.0f,					1.0f);
	return result;
}
Matrix3x3 Matrix3x3::BuildScale2D(F32 x, F32 y)
{
	Matrix3x3 result = Matrix3x3(	x,		0.0f,	0.0f,
									0.0f,	y,		0.0f,
									0.0f,	0.0f,	1.0f);
	return result;
}
