#pragma once

#include "Datatypes.h"
#include "Strings/String.h"
#include "Vector2.h"
#include "Vector3.h"

namespace LeEK
{
	class Matrix3x3
	{
	private:
		F32 values[9]; //VF32 values[4]?
	public:
		static const U32 MATRIX_SIZE = 3;
		static const Matrix3x3 Zero;
		static const Matrix3x3 Identity;

		Matrix3x3();
		Matrix3x3(	F32 v11, F32 v12, F32 v13, 
					F32 v21, F32 v22, F32 v23,
					F32 v31, F32 v32, F32 v33);
		Matrix3x3(	const Vector3& row1,
					const Vector3& row2,
					const Vector3& row3);
		#pragma region Operators
		inline F32& operator()(U32 row, U32 col)
		{
			return values[row + MATRIX_SIZE*col];
		}
		inline const F32& operator()(U32 row, U32 col)
		const {
			return values[row + MATRIX_SIZE*col];
		}
		inline Matrix3x3& operator*=(F32 rhs)
		{
			for(U32 i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++)
			{
				values[i] *= rhs;
			}
			return *this;
		}
		inline Matrix3x3& operator*=(const Matrix3x3& rhs)
		{
			Matrix3x3 result = Matrix3x3::Zero;
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
				}
			}
			*this = result;

			return *this;
		}
		inline Matrix3x3& operator/=(F32 rhs)
		{
			for(U32 i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++)
			{
				values[i] /= rhs;
			}
			return *this;
		}
	#pragma endregion

		String ToString();
		inline const F32* ToFloatArray() { return values; }
		Vector3 Multiply(const Vector3& v);
		inline Vector2 MultiplyPoint(const Vector2& v) 
		{
			Vector3 result = Multiply(Vector3(v.X(), v.Y(), 1.0f));
			return Vector2(result.X(), result.Y());
		}
		inline Vector2 MultiplyVector(const Vector2& v) 
		{
			Vector3 result = Multiply(Vector3(v.X(), v.Y(), 0.0f));
			return Vector2(result.X(), result.Y());
		}
		inline F32 Determinant()
		{
			return	(*this)(0,0)*((*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1)) -
					(*this)(0,1)*((*this)(1,0) * (*this)(2,2) - (*this)(1,2) * (*this)(2,0)) +
					(*this)(0,2)*((*this)(1,0) * (*this)(2,1) - (*this)(1,1) * (*this)(2,0));
		}
		inline void Transpose()
		{
			*this = this->GetTranspose();
		}
		Matrix3x3 GetTranspose()
		{
			Matrix3x3 ret;

			for(U32 i = 0; i < MATRIX_SIZE; i++)
			{
				for(U32 j = 0; j < MATRIX_SIZE; j++)
				{
					ret(i, j) = this->operator()(j, i);
				}
			}

			return ret;
		}

		//matrix building functions -
		//all have a similar structure to their respective 4x4 matrices,
		//but they operate in 2D space
		//TODO: add Vector2, and Vector2 overloads of these functions?
		static Matrix3x3 BuildTranslation(F32 x, F32 y);
		static Matrix3x3 BuildRotation2D(F32 angleRads);
		static Matrix3x3 BuildScale2D(F32 x, F32 y);
		static inline Matrix3x3 BuildUniformScale2D(F32 scaleFactor) { return BuildScale2D(scaleFactor, scaleFactor); }
	};

	#pragma region Static Operators
	inline Matrix3x3 operator *(Matrix3x3 lhs, const Matrix3x3& rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Matrix3x3 operator *(Matrix3x3 lhs, const F32 rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Matrix3x3 operator /(Matrix3x3 lhs, const F32 rhs)
	{
		lhs /= rhs;
		return lhs;
	}
	#pragma endregion
}
