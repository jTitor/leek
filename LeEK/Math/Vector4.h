//include guard
#pragma once
#include "Datatypes.h"
#include "Vector3.h"
#include "MathFunctions.h"
#include "Strings/String.h"

namespace LeEK
{
	//Vectors are row vectors!
	class Vector4
	{
	private:
		F32 x, y, z, w;
	public:
		//constants
		static const Vector4 Zero;
		static const Vector4 ZeroPoint;

		Vector4();
		Vector4(F32 x, F32 y, F32 z, F32 w);
		Vector4(const Vector3& v, F32 w);
		#pragma region Accessors
		inline F32 X() const { return x; }
		inline void SetX(F32 val) { x = val; }
		inline F32 Y() const { return y; }
		inline void SetY(F32 val) { y = val; }
		inline F32 Z() const { return z; }
		inline void SetZ(F32 val) { z = val; }
		inline F32 W() const { return w; }
		inline void SetW(F32 val) { w = val; }
		inline Vector3 XYZ() const
		{
			return Vector3(x, y, z);
		}
		inline void SetXYZ(const Vector3& xyz)
		{
			x = xyz.X();
			y = xyz.Y();
			z = xyz.Z();
		}
		#pragma endregion

		#pragma region Operators
		//no assignment operator?
		inline Vector4& operator +=(const Vector4& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;

			return *this;
		}
		inline Vector4& operator -=(const Vector4& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;

			return *this;
		}
		inline Vector4& operator *=(const F32 rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;

			return *this;
		}
		inline Vector4& operator /=(const F32 rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			w /= rhs;

			return *this;
		}
		inline Vector4 operator -()
		{
			return Vector4(-(*this).x, -(*this).y, -(*this).z, -(*this).w);
		}
		#pragma endregion

		String ToString() const;

		inline F32 Dot(const Vector4& rhs) const
		{
			return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
		}
		//inline Vector4 Cross(const Vector4& rhs)
		//{
			// a.Cross(b) = determinant of the form
			// |(basis)|
			// |   a   |
			// |   b   |
			// = <a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x>
		//	return Vector4(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
		//}

		//bool IsNaN();
	
		static inline F32 Distance(const Vector4& a, const Vector4& b) { return Math::Sqrt(DistanceSquared(a, b)); }
		static inline F32 DistanceSquared(const Vector4& a, const Vector4& b)
		{
			Vector4 delta = Vector4(b).operator-=(a);
			return delta.LengthSquared();
		}
		//{
		//	Vector4 lhs = b;
		//	//Vector4 rhs = a;
		//	//still don't understand why everything's borked
		//	Vector4 delta = lhs.operator-=(a);
		//	return delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
		//}
		//static inline float SignedDistanceSquared(const Vector4& a, const Vector4& b);
	
		inline F32 Length() const { return Math::Sqrt(LengthSquared()); }
		inline F32 LengthSquared() const { return x*x + y*y + z*z + w*w; }
	
	
		/**
		 * @return
		 * the Vector2 as a float array = {x, y}.
		 */
		//public float[] ToFloatArray();
		inline void Normalize()
		{
			F32 sqrLen = (*this).LengthSquared();
			if(!Math::ApproxEqual(sqrLen, 1.0f))
			{
				(*this).operator/=(Math::Sqrt(sqrLen));
			}
		}
		inline Vector4 GetNormalized()
		{
			Vector4 result = *this;
			result.Normalize();
			return result; 
		}
	};

	//comparison operators
	inline bool operator==(const Vector4& lhs, const Vector4& rhs){ return  Math::ApproxEqual(lhs.X(), rhs.X()) &&
																			Math::ApproxEqual(lhs.Y(), rhs.Y()) && 
																			Math::ApproxEqual(lhs.Z(), rhs.Z()) &&
																			Math::ApproxEqual(lhs.W(), rhs.W()); } 
	inline bool operator!=(const Vector4& lhs, const Vector4& rhs){ return !operator==(lhs,rhs); }
	//don't get why I have to do a const_cast here...
	inline bool operator<(const Vector4& lhs, const Vector4& rhs){ return lhs.LengthSquared() < rhs.LengthSquared(); } 
	inline bool operator>(const Vector4& lhs, const Vector4& rhs){ return lhs.LengthSquared() > rhs.LengthSquared(); }
	inline bool operator<=(const Vector4& lhs, const Vector4& rhs){ return !operator>(lhs,rhs); }
	inline bool operator>=(const Vector4& lhs, const Vector4& rhs){ return !operator<(lhs,rhs); }


	#pragma region Static Operators
	inline const Vector4 operator +(Vector4 lhs, const Vector4& rhs)
	{
		return Vector4(lhs += rhs);
	}
	inline const Vector4 operator -(Vector4 lhs, const Vector4& rhs)
	{
		return Vector4(lhs -= rhs);
	}
	inline const Vector4 operator *(const F32& lhs, Vector4 rhs)
	{
		return Vector4(rhs *= lhs);
	}
	inline const Vector4 operator /(const F32& lhs, Vector4 rhs)
	{
		return Vector4(rhs /= lhs);
	}
	inline const Vector4 operator *(Vector4 lhs, const F32& rhs)
	{
		return Vector4(lhs *= rhs);
	}
	inline const Vector4 operator /(Vector4 lhs, const F32& rhs)
	{
		return Vector4(lhs /= rhs);
	}
	inline const Vector4 operator -(const Vector4& other)
	{
		return -1.0f*other;
	}
	#pragma endregion 
}
