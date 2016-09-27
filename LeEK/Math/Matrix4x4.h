//include guard
#pragma once
#include "Datatypes.h"
//#include "Vector3.h"
#include "Matrix3x3.h"
#include "Vector4.h"
#include "Strings/String.h"

//4x4 matrix.
namespace LeEK
{
	/**
	*	Class for a 4x4 matrix.
	*	The data is in a 1D array, stored in row major order.
	*	The data is sectioned as follows:
	*	[	X.x	X.y	X.z	T.x	]
	*	[	Y.x	Y.y	Y.z	T.y	]
	*	[	Z.x	Z.y	Z.z	T.z	]
	*	[	0	0	0	1	]
	*	Where X is the matrix's X basis vector,
	*	Y is the Y basis,
	*	Z is the Z basis,
	*	and T is the translation vector.
	*/
	class Matrix4x4
	{
	private:
		//data is in a 1D array, stored in column major order so we don't have to transpose (OpenGL and DirectX both expect row-vector based matrices)
		//the indices relate to matrix positions as follows:
		//[	0	1	2	3	]
		//[	4	5	6	7	]
		//[	8	9	10	11	]
		//[	12	13	14	15	]
		F32 values[16]; //VF32 values[4]?
	public:
		static const U32 MATRIX_SIZE = 4;
		static const Matrix4x4 Zero;
		static const Matrix4x4 Identity;

		Matrix4x4();
		Matrix4x4(	F32 v11, F32 v12, F32 v13, F32 v14, 
					F32 v21, F32 v22, F32 v23, F32 v24,
					F32 v31, F32 v32, F32 v33, F32 v34,
					F32 v41, F32 v42, F32 v43, F32 v44);
		Matrix4x4(	const Vector4& row1,
					const Vector4& row2,
					const Vector4& row3,
					const Vector4& row4);
		Matrix4x4( const Matrix3x3& rotComponent, const Vector3& transComponent);
		//Matrix4x4::Matrix4x4(const Matrix3x3& rot, const Vector3& trans = Vector3::Zero, F32 scale = 1.0f);
		#pragma region Operators
		inline F32& operator()(const U32 row, const U32 col)
		{
			return values[row + MATRIX_SIZE*col];
		}
		inline const F32& operator()(U32 row, U32 col) const 
		{ 
			return values[row + MATRIX_SIZE*col];
		}
		inline Matrix4x4& operator*=(F32 rhs)
		{
			for(U32 i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++)
			{
				values[i] *= rhs;
			}
			return *this;
		}
		inline Matrix4x4& operator/=(F32 rhs)
		{
			for(U32 i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++)
			{
				values[i] /= rhs;
			}
			return *this;
		}
		inline Matrix4x4& operator*=(const Matrix4x4& rhs)
		{
			//PROFILE("MatrixMult");
			Matrix4x4 result = Matrix4x4::Zero;
			for(U32 i = 0; i < MATRIX_SIZE; i++)
			{
				for(U32 j = 0; j < MATRIX_SIZE; j++)
				{
					//F32 temp = (*this)(i, j);
					//(*this)(i, j) = 0.0f;
					for(U32 k = 0; k < MATRIX_SIZE; k++)
					{
						result(i, j) += (*this)(i, k) * rhs(k, j);
					}
					//(*this)(i, j) =	(*this)(i, 0) * const_cast<Matrix4x4&>(rhs)(0, j) + 
					//				(*this)(i, 1) * const_cast<Matrix4x4&>(rhs)(1, j) +
					//				(*this)(i, 2) * const_cast<Matrix4x4&>(rhs)(2, j) +
					//				(*this)(i, 3) * const_cast<Matrix4x4&>(rhs)(3, j);
				}
			}
			*this = result;

			return *this;
		}
		#pragma endregion

		String ToString() const;
		inline const F32* ToFloatArray() const { return values; }
		Vector4 GetRow(U32 rowInd) const;
		Vector4 GetCol(U32 colInd) const;
		void CopyToBuffer(F32* buffer) const;
		Vector4 MultiplyVector4(const Vector4& v) const;
		inline Vector3 MultiplyPoint(const Vector3& v) const
		{
			Vector4 tempResult = Matrix4x4::MultiplyVector4(Vector4(Vector3(v), 1.0f));
			return Vector3(tempResult.X(), tempResult.Y(), tempResult.Z());
		}
		inline Vector3 MultiplyPerspPoint(const Vector3& v) const
		{
			Vector4 tempResult = Matrix4x4::MultiplyVector4(Vector4(Vector3(v), 1.0f));
			if(Math::ApproxEqual(tempResult.W(), 0.0f))
			{
				tempResult.SetW(FLT_MIN);
			}

			return Vector3(tempResult.X(), tempResult.Y(), tempResult.Z()) / tempResult.W();
		}
		inline Vector3 MultiplyVector(const Vector3& v) const
		{
			Vector4 tempResult = Matrix4x4::MultiplyVector4(Vector4(Vector3(v), 0.0f));
			return Vector3(tempResult.X(), tempResult.Y(), tempResult.Z());
		}
		Matrix4x4 FindInverse() const;
		//If the matrix is a transformation matrix,
		//will get its inverse (probably) faster than using FindInverse().
		//Don't use this if the matrix isn't a transformation matrix, or it'll be completely invalid!
		Matrix4x4 GetTransformInverse() const;
		Matrix3x3 GetRotationSubmatrix() const
		{
			return Matrix3x3(	(*this)(0,0),	(*this)(0,1),	(*this)(0,2),
								(*this)(1,0),	(*this)(1,1),	(*this)(1,2),
								(*this)(2,0),	(*this)(2,1),	(*this)(2,2));
		}
		Vector3 GetTranslateComponent() const
		{
			return Vector3((*this)(0,3), (*this)(1,3), (*this)(2,3));
		}
		inline void Transpose()
		{
			*this = this->GetTranspose();
		}
		Matrix4x4 GetTranspose() const
		{
			Matrix4x4 ret;

			for(U32 i = 0; i < MATRIX_SIZE; i++)
			{
				for(U32 j = 0; j < MATRIX_SIZE; j++)
				{
					ret(i, j) = this->operator()(j, i);
				}
			}

			return ret;
		}
		Matrix3x3 GetSubmatrix(U32 row, U32 col) const;
		F32 Determinant() const;
		Matrix4x4 Adjoint() const;

		#pragma region Matrix Building Functions
		static inline Matrix4x4 BuildTranslation(const Vector3& v) { return BuildTranslation(v.X(), v.Y(), v.Z()); }
		//acts as a combination of (X * Y * Z) rotation matrices, in that order
		static Matrix4x4 BuildTranslation(F32 x, F32 y, F32 z);
		static inline Matrix4x4 FromEulerAngles(const Vector3& eulerAngles) { return FromEulerAngles(eulerAngles.X(), eulerAngles.Y(), eulerAngles.Z()); }
		static Matrix4x4 FromEulerAngles(F32 xRads, F32 yRads, F32 zRads);
		static Matrix4x4 BuildXRotation(F32 angleRads); //{ return FromEulerAngles(angleRads, 0.0f, 0.0f); }
		static Matrix4x4 BuildYRotation(F32 angleRads); //{ return FromEulerAngles(0.0f, angleRads, 0.0f); }
		static Matrix4x4 BuildZRotation(F32 angleRads); //{ return FromEulerAngles(0.0f, 0.0f, angleRads); }
		static Matrix4x4 BuildScale(F32 x, F32 y, F32 z);
		static Matrix4x4 BuildViewRH(const Vector3& cameraPos, const Vector3& lookDirection, const Vector3& up);
		static Matrix4x4 BuildViewLH(const Vector3& cameraPos, const Vector3& lookDirection, const Vector3& up);
		static Matrix4x4 BuildViewLookAtRH(const Vector3& cameraPos, const Vector3& lookAt, const Vector3& up);
		static Matrix4x4 BuildViewLookAtLH(const Vector3& cameraPos, const Vector3& lookAt, const Vector3& up);
		static Matrix4x4 BuildPerspectiveRH(F32 aspectRatio, F32 fovRads, F32 nearDist, F32 farDist);
		static Matrix4x4 BuildPerspectiveLH(F32 aspectRatio, F32 fovRads, F32 nearDist, F32 farDist);
		static Matrix4x4 BuildOrthographicRH(F32 top, F32 bottom, F32 left, F32 right, F32 nearDist, F32 farDist);
		static Matrix4x4 BuildOrthographicLH(F32 top, F32 bottom, F32 left, F32 right, F32 nearDist, F32 farDist);
		static inline Matrix4x4 BuildScale(F32 scaleFactor) { return BuildScale(scaleFactor, scaleFactor, scaleFactor); }
		//Matrix4x4 BuildRotationAboutAxis(Vector3& axis, F32 angleRads);
		#pragma endregion
	};

#pragma region Static Operators
	inline Matrix4x4 operator *(Matrix4x4 lhs, const Matrix4x4& rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Matrix4x4 operator *(Matrix4x4 lhs, const F32 rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Matrix4x4 operator /(Matrix4x4 lhs, const F32 rhs)
	{
		lhs /= rhs;
		return lhs;
	}
#pragma endregion
}
