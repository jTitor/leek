//include guard
#pragma once
#include "Datatypes.h"
#include "MathFunctions.h"
#include "Strings/String.h"

namespace LeEK
{
	class Vector3 //: protected v3Data
	{
	private:
		static const U32 NUM_COMPONENTS = 3;
		F32 tuple[NUM_COMPONENTS];
	public:
		//constants
		static const Vector3 Zero;
		static const Vector3 One;
		static const Vector3 Up;
		static const Vector3 Right;
		static const Vector3 Forward;

		Vector3();
		Vector3(F32 x, F32 y, F32 z);
		#pragma region Accessors
		inline F32 X() const { return tuple[0]; }
		inline void SetX(F32 val) { tuple[0] = val; }
		inline F32 Y() const { return tuple[1]; }
		inline void SetY(F32 val) { tuple[1] = val; }
		inline F32 Z() const { return tuple[2]; }
		inline void SetZ(F32 val) { tuple[2] = val; }
		#pragma endregion

		#pragma region Operators
		//no assignment operator?
		inline Vector3& operator +=(const Vector3& rhs)
		{
			tuple[0] += rhs.tuple[0];
			tuple[1] += rhs.tuple[1];
			tuple[2] += rhs.tuple[2];

			return *this;
		}
		inline Vector3& operator -=(const Vector3& rhs)
		{
			tuple[0] -= rhs.tuple[0];
			tuple[1] -= rhs.tuple[1];
			tuple[2] -= rhs.tuple[2];

			return *this;
		}
		//this is smaller than the size of a void* under x64,
		//so pass by ref would be a bad idea
		inline Vector3& operator *=(const F32 rhs)
		{
			tuple[0] *= rhs;
			tuple[1] *= rhs;
			tuple[2] *= rhs;

			return *this;
		}
		inline Vector3& operator /=(const F32 rhs)
		{
			tuple[0] /= rhs;
			tuple[1] /= rhs;
			tuple[2] /= rhs;

			return *this;
		}
		inline Vector3 operator -()
		{
			return Vector3(-(*this).tuple[0], -(*this).tuple[1], -(*this).tuple[2]);
		}
		#pragma endregion 

		String ToString() const;
		const F32* ToFloatArray() const;
		
		inline F32 Dot(const Vector3 rhs) const
		{
			return tuple[0]*rhs.tuple[0] + tuple[1]*rhs.tuple[1] + tuple[2]*rhs.tuple[2];
		}
		inline Vector3 Cross(const Vector3& rhs) const 
		{
			// a.Cross(b) = determinant of the form
			// |(basis)|
			// |   a   |
			// |   b   |
			// = <	a.tuple[1]*b.tuple[2] - a.tuple[2]*b.tuple[1], 
			//		a.tuple[2]*b.tuple[0] - a.tuple[0]*b.tuple[2],
			//		a.tuple[0]*b.tuple[1] - a.tuple[1]*b.tuple[0] >
			return Vector3(	tuple[1]*rhs.tuple[2] - tuple[2]*rhs.tuple[1], 
							tuple[2]*rhs.tuple[0] - tuple[0]*rhs.tuple[2],
							tuple[0]*rhs.tuple[1] - tuple[1]*rhs.tuple[0]);
		}
	
		inline F32 Length() const 
		{ 
			//PROFILE("Vec3Len");
			return Math::Sqrt(LengthSquared()); 
		}
		inline F32 LengthSquared() const
		{
			return tuple[0]*tuple[0] + tuple[1]*tuple[1] + tuple[2]*tuple[2];
		}

		/**
		* Normalizes a vector; zero vectors normalize to a zero vector.
		*/
		inline void Normalize()
		{
			//PROFILE("Vec3Norm");
			F32 sqrLen = (*this).LengthSquared();
			//handle edge case
			if(sqrLen == 0.0f)
			{
				return;
			}
			if(!Math::ApproxEqual(sqrLen, 1.0f))
			{
				(*this).operator/=(Math::Sqrt(sqrLen));
			}
		}
		inline Vector3 GetNormalized() 
		const { 
			Vector3 result(*this);
			result.Normalize();
			return result; 
		}

		/**
		Gets this vector, with all negative components
		set to their respective positive value.
		*/
		inline Vector3 GetAbs() const
		{
			return Vector3(	Math::Abs(tuple[0]),
							Math::Abs(tuple[1]), 
							Math::Abs(tuple[2]));
		}

		/**
		Gets the value of the largest component in this vector.
		*/
		inline F32 GetMaxComponent() const
		{
			return Math::Max(Math::Max(tuple[0], tuple[1]), tuple[2]);
		}

		/**
		Gets the value of the smallest component in this vector.
		*/
		inline F32 GetMinComponent() const
		{
			return Math::Min(Math::Min(tuple[0], tuple[1]), tuple[2]);
		}

		inline Vector3 ComponentwiseProduct(const Vector3& rhs) const
		{
			return Vector3(	tuple[0] * rhs.tuple[0],
							tuple[1] * rhs.tuple[1],
							tuple[2] * rhs.tuple[2]);
		}

		/**
		Gets the dot product - the sum of a componentwise multiplication between two vectors.
		*/
		static inline F32 Dot(const Vector3& lhs, const Vector3& rhs)
		{
			return lhs.Dot(rhs);
		}

		/**
		Gets a cross product - a vector perpendictular to two given vectors.
		*/
		static inline Vector3 Cross(const Vector3& lhs, const Vector3& rhs)
		{
			return lhs.Cross(rhs);
		}

		//bool IsNaN();
	
		/**
		Gets the distance between two points.
		*/
		static inline F32 Distance(const Vector3& a, const Vector3& b)
		{ 
			//PROFILE("Vec3Dist");
			return Math::Sqrt(DistanceSquared(a, b)); 
		}
		/**
		Gets the squared distance between two points;
		Generally it's faster to compare squared distances
		since you don't have to take the square root of a number.
		*/
		static inline F32 DistanceSquared(const Vector3& a, const Vector3& b)
		{
			Vector3 delta = Vector3(b).operator-=(a);
			return delta.LengthSquared();
		}

		/**
		Projects one vector onto another vector.
		Note that neither vector need be normalized;
		if you know that a destination is normalized, use ProjectToNormalized.
		@param sourceVec the vector to project from.
		@param destVec the vector to project to.
		*/
		static inline Vector3 Project(const Vector3& sourceVec, const Vector3& destVec)
		{
			Vector3 destNorm = destVec.GetNormalized();
			return ProjectToNormalized(sourceVec, destNorm);
		}

		/**
		Projects one vector onto a NORMALIZED vector.
		*/
		static inline Vector3 ProjectToNormalized(const Vector3& sourceVec, Vector3 destNorm)
		{
			F32 projLen = sourceVec.Dot(destNorm);
			return Vector3(destNorm *= projLen);
		}

		static inline Vector3 ComponentwiseProduct(const Vector3& lhs, const Vector3& rhs)
		{
			return lhs.ComponentwiseProduct(rhs);
		}

		/**
		Ensures that the given vectors are the min and max coordinates of a box.
		If any components violate this rule, they are swapped between the two vectors.
		*/
		static inline void MakeBoxCorners(Vector3* min, Vector3* max)
		{
			for(U32 i = 0; i < NUM_COMPONENTS; ++i)
			{
				F32 minVal = min->tuple[i];
				F32 maxVal = max->tuple[i];

				if(minVal > maxVal)
				{
					min->tuple[i] = maxVal;
					max->tuple[i] = minVal;
				}
			}
		}
	};

	#pragma region Comparison Operators
	inline bool operator==(const Vector3& lhs, const Vector3& rhs){ return  Math::ApproxEqual(lhs.X(), rhs.X()) &&
																			Math::ApproxEqual(lhs.Y(), rhs.Y()) && 
																			Math::ApproxEqual(lhs.Z(), rhs.Z()); } 
	inline bool operator!=(const Vector3& lhs, const Vector3& rhs){ return !operator==(lhs,rhs); }
	inline bool operator<(const Vector3& lhs, const Vector3& rhs){ return lhs.LengthSquared() < rhs.LengthSquared(); } 
	inline bool operator>(const Vector3& lhs, const Vector3& rhs){ return lhs.LengthSquared() > rhs.LengthSquared(); }
	inline bool operator<=(const Vector3& lhs, const Vector3& rhs){ return !operator>(lhs,rhs); }
	inline bool operator>=(const Vector3& lhs, const Vector3& rhs){ return !operator<(lhs,rhs); }
	#pragma endregion 

	#pragma region Static Operators
	inline const Vector3 operator +(Vector3 lhs, const Vector3& rhs)
	{
		return Vector3(lhs += rhs);
	}
	inline const Vector3 operator -(Vector3 lhs, const Vector3& rhs)
	{
		return Vector3(lhs -= rhs);
	}
	inline const Vector3 operator *(const F32& lhs, Vector3 rhs)
	{
		return Vector3(rhs *= lhs);
	}
	inline const Vector3 operator /(const F32& lhs, Vector3 rhs)
	{
		return Vector3(rhs /= lhs);
	}
	inline const Vector3 operator *(Vector3 lhs, const F32& rhs)
	{
		return Vector3(lhs *= rhs);
	}
	inline const Vector3 operator /(Vector3 lhs, const F32& rhs)
	{
		return Vector3(lhs /= rhs);
	}
	inline const Vector3 operator -(const Vector3& other)
	{
		return -1.0f*other;
	}
	#pragma endregion 
}
