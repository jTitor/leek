#include "StdAfx.h"
//#include "MathFunctions.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"

using namespace LeEK;

const Matrix4x4 Matrix4x4::Zero(	0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f);
const Matrix4x4 Matrix4x4::Identity(	1.0f, 0.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f, 0.0f,
										0.0f, 0.0f, 1.0f, 0.0f,
										0.0f, 0.0f, 0.0f, 1.0f);

Matrix4x4::Matrix4x4()
{
	Matrix4x4(	0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f);
}

Matrix4x4::Matrix4x4(	F32 v11, F32 v12, F32 v13, F32 v14, 
						F32 v21, F32 v22, F32 v23, F32 v24,
						F32 v31, F32 v32, F32 v33, F32 v34,
						F32 v41, F32 v42, F32 v43, F32 v44)
{
	(*this)(0,0) = v11;
	(*this)(0,1) = v12;
	(*this)(0,2) = v13;
	(*this)(0,3) = v14;

	(*this)(1,0) = v21;
	(*this)(1,1) = v22;
	(*this)(1,2) = v23;
	(*this)(1,3) = v24;

	(*this)(2,0) = v31;
	(*this)(2,1) = v32;
	(*this)(2,2) = v33;
	(*this)(2,3) = v34;

	(*this)(3,0) = v41;
	(*this)(3,1) = v42;
	(*this)(3,2) = v43;
	(*this)(3,3) = v44;
}

Matrix4x4::Matrix4x4(	const Vector4& row1,
						const Vector4& row2,
						const Vector4& row3,
						const Vector4& row4)
{
	Matrix4x4(	row1.X(),	row1.Y(),	row1.Z(),	row1.W(),
				row2.X(),	row2.Y(),	row2.Z(),	row2.W(),
				row3.X(),	row3.Y(),	row3.Z(),	row3.W(),
				row4.X(),	row4.Y(),	row4.Z(),	row4.W());
}

Matrix4x4::Matrix4x4(const Matrix3x3& rot, const Vector3& trans)
{
	Matrix4x4(	rot(0,0),	rot(0,1),	rot(0,2),	trans.X(),
				rot(1,0),	rot(1,1),	rot(1,2),	trans.Y(),
				rot(2,0),	rot(2,1),	rot(2,2),	trans.Z(),
				0,			0,			0,			1.0f);
}

String Matrix4x4::ToString() const
{
	String resStr = "\n";
	char rowBuf[192];
	for(U32 i = 0; i < MATRIX_SIZE; i++)
	{
		//print a row of the matrix
		sprintf_s(rowBuf, sizeof(rowBuf), "\t[ %.4f, %.4f, %.4f, %.4f ]\n", (*this)(i, 0), (*this)(i, 1), (*this)(i, 2), (*this)(i, 3));
		//sstrm << "\t[ " << (*this)(i, 0) << ", " << (*this)(i, 1) << ", " << (*this)(i, 2) << " ]" << "\n";
		resStr += rowBuf;
	}
	return resStr;
}

Vector4 Matrix4x4::GetRow(U32 rowInd) const
{
	if(rowInd >= MATRIX_SIZE)
	{
		return Vector4::Zero;
	}

	return Vector4(	this->operator()(rowInd, 0),
					this->operator()(rowInd, 1),
					this->operator()(rowInd, 2),
					this->operator()(rowInd, 3));
}

Vector4 Matrix4x4::GetCol(U32 colInd) const
{
	if(colInd >= MATRIX_SIZE)
	{
		return Vector4::Zero;
	}

	return Vector4(	this->operator()(0, colInd),
					this->operator()(1, colInd),
					this->operator()(2, colInd),
					this->operator()(3, colInd));
}

void Matrix4x4::CopyToBuffer(F32* buffer) const
{
	for(U32 i = 0; i < MATRIX_SIZE*MATRIX_SIZE; i++)
	{
		buffer[i] = values[i];
	}
}

Vector4 Matrix4x4::MultiplyVector4(const Vector4& v) const
{
	return Vector4(	v.X() * (*this)(0, 0) + v.Y() * (*this)(1, 0) + v.Z() * (*this)(2, 0) + v.W() * (*this)(3, 0),
					v.X() * (*this)(0, 1) + v.Y() * (*this)(1, 1) + v.Z() * (*this)(2, 1) + v.W() * (*this)(3, 1),
					v.X() * (*this)(0, 2) + v.Y() * (*this)(1, 2) + v.Z() * (*this)(2, 2) + v.W() * (*this)(3, 2),
					v.X() * (*this)(0, 3) + v.Y() * (*this)(1, 3) + v.Z() * (*this)(2, 3) + v.W() * (*this)(3, 3));
}

Matrix3x3 Matrix4x4::GetSubmatrix(U32 row, U32 col) const
{
	U32 nextRow = 0;
	U32 nextColumn = 0;
	Matrix3x3 result = Matrix3x3::Zero;

	//we will loop through each element of our 4x4;
	//if the current element is valid (neither on the same row as [row] or column as [col])
	//then set Matrix3x3(nextRow, nextColumn) and increment nextColumn
	//then increment nextRow
	//the last step might not be necessary
	for(U32 i = 0; i < MATRIX_SIZE; i++)
	{
		//if this row is valid...
		if(i != row)
		{
			//reset nextColumn
			nextColumn = 0;
			for(U32 j = 0; j < MATRIX_SIZE; j++)
			{
				if(j != col)
				{
					result(nextRow, nextColumn) = (*this)(i, j);
					nextColumn++;
				}
			}
			//move nextRow down a row
			nextRow++;
		}
	}
	return result;
}

F32 Matrix4x4::Determinant() const
{
	//naming format derived from this matrix:
	//[	a	b	c	d	]
	//[	e	f	g	h	]
	//[	i	j	k	l	]
	//[	m	n	o	p	]
	//Painful Way:
	//calculate subdeterminants
	//
	//so klop =	|	k	l	|
	//			|	o	p	|

	//Easy, But Maybe Slow Way:
	//Take submatrices at a, b, c, and d, then take their determinants
	float result = 0.0f;
	for(U32 i = 0; i < MATRIX_SIZE; i++)
	{
		result += pow(-1.0f, i) * (*this)(0, i) * GetSubmatrix(0, i).Determinant();
	}
	return result;
}

Matrix4x4 Matrix4x4::Adjoint() const
{
	//another huge pain!
	//build a new 4x4 matrix from cofactors.
	//C(i, j) = (-1)^(i+j)*det(submatrix(i,j))
	Matrix4x4 result = Matrix4x4::Zero;
	for(U32 i = 0; i < MATRIX_SIZE; i++)
	{
		for(U32 j = 0; j < MATRIX_SIZE; j++)
		{
			result(i, j) = pow(-1.0f, i + j) * GetSubmatrix(i, j).Determinant();
		}
	}

	return result.GetTranspose();
}

Matrix4x4 Matrix4x4::FindInverse() const
{
	//Cramer's rule: inverse = adjoint / determinant
	float det = Determinant();

	//if matrix can't be inverted, return a zero matrix
	if(Math::ApproxEqual(det, 0.0f))
	{
		return Matrix4x4::Zero;
	}

	//otherwise, calculate the inverse
	return Adjoint() / det;
}

Matrix4x4 Matrix4x4::GetTransformInverse() const
{
	Matrix3x3 rotComponent = this->GetRotationSubmatrix();
	rotComponent.Transpose();
	Vector3 transComponent = this->GetTranslateComponent();
	transComponent = -rotComponent.Multiply(transComponent);
	Matrix4x4 result = Matrix4x4(rotComponent, transComponent);
	return result;
}

#pragma region Matrix Building Functions
Matrix4x4 Matrix4x4::BuildTranslation(F32 x, F32 y, F32 z) 
{
	//Matrix4x4 result = Matrix4x4(	1.0f,	0.0f,	0.0f,	0.0f,
	//								0.0f,	1.0f,	0.0f,	0.0f,
	//								0.0f,	0.0f,	1.0f,	0.0f,
	//								x,		y,		z,		1.0f);
	Matrix4x4 result = Matrix4x4(	1.0f,	0.0f,	0.0f,	x,
									0.0f,	1.0f,	0.0f,	y,
									0.0f,	0.0f,	1.0f,	z,
									0.0f,	0.0f,	0.0f,	1.0f);
	return result;
}
Matrix4x4 Matrix4x4::FromEulerAngles(F32 xRads, F32 yRads, F32 zRads)
{
	//compute the base cosines
	F32 cosX = Math::Cos(xRads);
	F32 cosY = Math::Cos(yRads);
	F32 cosZ = Math::Cos(zRads);
	F32 sinX = Math::Sin(xRads);
	F32 sinY = Math::Sin(yRads);
	F32 sinZ = Math::Sin(zRads);

	Matrix4x4 result = Matrix4x4(	cosY*cosZ,						-(cosY*sinZ),					sinY,			0.0f,
									sinX*sinY*cosZ + cosX*sinZ,		-(sinX*sinY*sinZ) + cosX*cosZ,	-(sinX*cosY),	0.0f,
									-(cosX*sinY*cosZ) + sinX*sinZ,	cosX*sinY*sinZ + sinX*cosZ,		cosX*cosY,		0.0f,
									0.0f,							0.0f,							0.0f,			1.0f);
	return result;
}

Matrix4x4 Matrix4x4::BuildXRotation(F32 angleRads)
{
	F32 cos = Math::Cos(angleRads);
	F32 sin = Math::Sin(angleRads);
	Matrix4x4 result = Matrix4x4(	1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	cos,	-sin,	0.0f,
									0.0f,	sin,	cos,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f);
	return result;
}
Matrix4x4 Matrix4x4::BuildYRotation(F32 angleRads)
{
	F32 cos = Math::Cos(angleRads);
	F32 sin = Math::Sin(angleRads);
	Matrix4x4 result = Matrix4x4(	cos,	0.0f,	sin,	0.0f,
									0.0f,	1.0f,	0.0f,	0.0f,
									-sin,	0.0f,	cos,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f);
	return result;
}
Matrix4x4 Matrix4x4::BuildZRotation(F32 angleRads)
{
	F32 cos = Math::Cos(angleRads);
	F32 sin = Math::Sin(angleRads);
	Matrix4x4 result = Matrix4x4(	cos,	-sin,	0.0f,	0.0f,
									sin,	cos,	0.0f,	0.0f,
									0.0f,	0.0f,	1.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f);
	return result;
}

Matrix4x4 Matrix4x4::BuildScale(F32 x, F32 y, F32 z)
{
	Matrix4x4 result = Matrix4x4(	x,		0.0f,	0.0f,	0.0f,
									0.0f,	y,		0.0f,	0.0f,
									0.0f,	0.0f,	z,		0.0f,
									0.0f,	0.0f,	0.0f,	1.0f);
	return result;
}

//constructs a world-to-view matrix.
//it's an affine matrix, so the inverse should be very easy to compute
Matrix4x4 Matrix4x4::BuildViewRH(const Vector3& eye, const Vector3& lookDirection, const Vector3& up)
{
	//don't completely understand derivation of this matrix - see Verth & Bishop, Chapter 6 (Viewing and Projection)
	//but it's the inverse of the view-to-world matrix; inverses of affine matrices are composed in the format
	//[	inverse(R)	-(inverse(R)*translation)	]
	//[	trans(0)	1							]
	//where R is the 3x3 rotation component of the matrix, and translation is the translation vector component of the matrix
	//since VtW is orthogonal, this becomes
	//[	trans(R)	-(trans(R)*translation)	]
	//[	trans(0)	1						]

	//what we'll do is build a rotation matrix, multiply the camera position (eye) by it, and then manually set values
	//Matrix3x3 rotation;
	//we'll need a basis for the matrix, starting with the camera's view direction (composed from lookAt-eye) and moving from there
	Vector3 viewDirection = -lookDirection.GetNormalized();
	Vector3 viewSide = up.Cross(viewDirection).GetNormalized();
	Vector3 viewUp = viewDirection.Cross(viewSide);

	//we might need to transpose the rotation component
	/*return Matrix4x4(	-viewSide.X(),			-viewSide.Y(),		-viewSide.Z(),		1.0f * (viewSide.Dot(eye)),
						-viewUp.X(),				-viewUp.Y(),			-viewUp.Z(),			1.0f * (viewUp.Dot(eye)),
						-viewDirection.X(),		-viewDirection.Y(),	-viewDirection.Z(),	1.0f * (viewDirection.Dot(eye)),
						0.0f,					0.0f,				0.0f,				1.0f);
	*/

	return Matrix4x4(	viewSide.X(),			viewSide.Y(),		viewSide.Z(),		-1.0f * (viewSide.Dot(eye)),
						viewUp.X(),				viewUp.Y(),			viewUp.Z(),			-1.0f * (viewUp.Dot(eye)),
						viewDirection.X(),		viewDirection.Y(),	viewDirection.Z(),	-1.0f * (viewDirection.Dot(eye)),
						0.0f,					0.0f,				0.0f,				1.0f);
}

Matrix4x4 Matrix4x4::BuildViewLH(const Vector3& eye, const Vector3& lookDirection, const Vector3& up)
{
	Vector3 viewDirection = lookDirection.GetNormalized();
	//Vector3 viewUp = (up - (viewDirection * up.Dot(viewDirection))).GetNormalized();
	//Vector3 viewSide = viewDirection.Cross(viewUp);
	Vector3 viewSide = up.Cross(viewDirection).GetNormalized();//viewDirection.Cross(up).GetNormalized();//up.Cross(viewDirection).GetNormalized();
	Vector3 viewUp = viewDirection.Cross(viewSide);//viewSide.Cross(viewDirection);//viewDirection.Cross(viewSide);

	//we might need to transpose the rotation component
	return Matrix4x4(	viewSide.X(),			viewSide.Y(),		viewSide.Z(),		-1.0f * (viewSide.Dot(eye)),
						viewUp.X(),				viewUp.Y(),			viewUp.Z(),			-1.0f * (viewUp.Dot(eye)),
						viewDirection.X(),		viewDirection.Y(),	viewDirection.Z(),	-1.0f * (viewDirection.Dot(eye)),
						0.0f,					0.0f,				0.0f,				1.0f);
}


Matrix4x4 Matrix4x4::BuildViewLookAtRH(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
{
	Vector3 viewDirection = (lookAt - eye).GetNormalized();
	return BuildViewRH(eye, viewDirection, up);
}

Matrix4x4 Matrix4x4::BuildViewLookAtLH(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
{
	Vector3 viewDirection = (lookAt - eye).GetNormalized();
	return BuildViewLH(eye, viewDirection, up);
}

Matrix4x4 Matrix4x4::BuildPerspectiveRH(F32 aspectRatio, F32 fovRads, F32 nearDist, F32 farDist)
{
	//don't completely understand derivation of this matrix - see Verth & Bishop, Chapter 6 (Viewing and Projection).

	//perspective matrix depends on division by distance between near and far planes, so quit if they're equal
	if(Math::ApproxEqual(nearDist, farDist) || Math::ApproxEqual(aspectRatio, 0.0f))
	{
		return Matrix4x4::Zero;
	}
	F32 distToProjPlane = 1.0f / Math::Tan(fovRads / 2.0f);
	F32 viewLenRecip = 1.0f / (nearDist - farDist);
	/*
	return Matrix4x4(	distToProjPlane / aspectRatio,	0.0f,				0.0f,											0.0f,
						0.0f,							distToProjPlane,	0.0f,											0.0f,
						0.0f,							0.0f,				(nearDist + farDist) * viewLenRecip,		(-2.0f * nearDist * farDist) * viewLenRecip,
						0.0f,							0.0f,				1.0f,											0.0f);
	*/
	return Matrix4x4(	distToProjPlane / aspectRatio,	0.0f,				0.0f,									0.0f,
						0.0f,							distToProjPlane,	0.0f,									0.0f,
						0.0f,							0.0f,				(nearDist + farDist) * viewLenRecip,	(2.0f * nearDist * farDist) * viewLenRecip,
						0.0f,							0.0f,				-1.0f,									0.0f);
}

Matrix4x4 Matrix4x4::BuildPerspectiveLH(F32 aspectRatio, F32 fovRads, F32 nearDist, F32 farDist)
{
	if(Math::ApproxEqual(nearDist, farDist))
	{
		return Matrix4x4::Zero;
	}

	F32 distToProjPlane = 1.0f / Math::Tan(fovRads / 2.0f);

	return Matrix4x4(	distToProjPlane / aspectRatio,	0.0f,				0.0f,								0.0f,
						0.0f,							distToProjPlane,	0.0f,								0.0f,
						0.0f,							0.0f,				farDist / (farDist - nearDist),		-(nearDist * farDist) / (farDist - nearDist),
						0.0f,							0.0f,				1.0f,								0.0f);
}

Matrix4x4 Matrix4x4::BuildOrthographicRH(F32 top, F32 bottom, F32 left, F32 right, F32 nearDist, F32 farDist)
{
	//don't understand derivation of this matrix - see Verth & Bishop, Chapter 6 (Viewing and Projection).
	if(Math::ApproxEqual(top, bottom) || Math::ApproxEqual(left, right) || Math::ApproxEqual(nearDist, farDist))
	{
		return Matrix4x4::Zero;
	}
	F32 heightRecip = 1.0f / (top - bottom);
	F32 widthRecip = 1.0f / (right - left);
	F32 viewLenRecip = 1.0f / (farDist - nearDist);

	return Matrix4x4(	2.0f * widthRecip,	0.0f,					0,						-(right + left) * widthRecip,
						0.0f,				2.0f * heightRecip,		0,						-(top + bottom) * heightRecip,
						0.0f,				0.0f,					-2.0f * viewLenRecip,	-(farDist + nearDist) * viewLenRecip,
						0.0f,				0.0f,					0.0f,					1.0f);
	/*return Matrix4x4(	2.0f * widthRecip,	0.0f,					widthRecip,				-(right + left - nearDist) * widthRecip,
						0.0f,				2.0f * heightRecip,		heightRecip,			-(top + bottom - nearDist) * heightRecip,
						0.0f,				0.0f,					-2.0f * viewLenRecip,	-(farDist + nearDist) * viewLenRecip,
						0.0f,				0.0f,					0.0f,					1.0f);*/
}

Matrix4x4 Matrix4x4::BuildOrthographicLH(F32 top, F32 bottom, F32 left, F32 right, F32 nearDist, F32 farDist)
{
	if(Math::ApproxEqual(top, bottom) || Math::ApproxEqual(left, right))
	{
		return Matrix4x4::Zero;
	}

	return Matrix4x4(	2.0f / (right - left),	0.0f,					0.0f,							-(right + left)/(right - left),
						0.0f,					2.0f / (top - bottom),	0.0f,							-(top + bottom)/(top - bottom),
						0.0f,					0.0f,					1.0f / (farDist - nearDist),	-(nearDist)/(farDist - nearDist),
						0.0f,					0.0f,					0.0f,							1.0f);
}


/*
Matrix4x4 Matrix4x4::BuildRotationAboutAxis(Vector3& axis, F32 angleRads)
{
}
*/
#pragma endregion
