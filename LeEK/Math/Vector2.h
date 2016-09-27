#pragma once

#include "Datatypes.h"
#include "MathFunctions.h"
#include "Strings/String.h"

namespace LeEK
{
	class Vector2
	{
	private:
		F32 x, y;
	//	F32 tuple[3];
	public:
		//constants
		static const Vector2 Zero;
		static const Vector2 One;
		static const Vector2 Up;
		static const Vector2 Right;

		Vector2() { Vector2(0.0f, 0.0f); };
		Vector2(F32 vx, F32 vy) { (*this).x = vx; (*this).y = vy; }
		~Vector2(void);
		#pragma region Accessors
		inline F32 X() const { return x; }
		inline void SetX(F32 val) { x = val; }
		inline F32 Y() const { return y; }
		inline void SetY(F32 val) { y = val; }
		#pragma endregion

		#pragma region Operators
		//no assignment operator?
		inline Vector2& operator +=(const Vector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;

			return *this;
		}
		inline Vector2& operator -=(const Vector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;

			return *this;
		}
		inline Vector2& operator *=(const F32 rhs)
		{
			x *= rhs;
			y *= rhs;

			return *this;
		}
		inline Vector2& operator /=(const F32 rhs)
		{
			x /= rhs;
			y /= rhs;

			return *this;
		}
		inline Vector2 operator -()
		{
			return Vector2(-(*this).x, -(*this).y);
		}
		#pragma endregion 

		String ToString() const;

		inline F32 Dot(const Vector2 rhs) const
		{
			return x*rhs.x + y*rhs.y;
		}
		static inline F32 Dot(const Vector2& lhs, const Vector2& rhs)
		{
			return lhs.Dot(rhs);
		}

		//bool IsNaN();
	
		static F32 Distance(const Vector2& a, const Vector2& b) { return Math::Sqrt(DistanceSquared(a, b)); };
		static F32 DistanceSquared(const Vector2& a, const Vector2& b)
		{
			Vector2 delta = Vector2(b).operator-=(a);
			return delta.LengthSquared();
		}
		//{
		//	Vector2 lhs = b;
		//	//Vector2 rhs = a;
		//	//still don't understand why everything's borked
		//	Vector2 delta = lhs.operator-=(a);
		//	return delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
		//}
		//static inline float SignedDistanceSquared(const Vector2& a, const Vector2& b);
	
		inline F32 Length() const { return Math::Sqrt(LengthSquared()); }
		inline F32 LengthSquared()
		const {
			return x*x + y*y;
		}
	
	
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
		inline Vector2 GetNormalized() 
		{ 
			Vector2 result = *this;
			result.Normalize();
			return result; 
		}
	};

	#pragma region Comparison Operators
	inline bool operator==(const Vector2& lhs, const Vector2& rhs){ return  Math::ApproxEqual(lhs.X(), rhs.X()) &&
																			Math::ApproxEqual(lhs.Y(), rhs.Y()); } 
	inline bool operator!=(const Vector2& lhs, const Vector2& rhs){ return !operator==(lhs,rhs); }
	//don't get why I have to do a const_cast here...
	inline bool operator<(const Vector2& lhs, const Vector2& rhs){ return lhs.LengthSquared() < rhs.LengthSquared(); } 
	inline bool operator>(const Vector2& lhs, const Vector2& rhs){ return lhs.LengthSquared() > rhs.LengthSquared(); }
	inline bool operator<=(const Vector2& lhs, const Vector2& rhs){ return !operator>(lhs,rhs); }
	inline bool operator>=(const Vector2& lhs, const Vector2& rhs){ return !operator<(lhs,rhs); }
	#pragma endregion 

	#pragma region Static Operators
	inline const Vector2 operator +(Vector2 lhs, const Vector2& rhs)
	{
		return Vector2(lhs += rhs);
	}
	inline const Vector2 operator -(Vector2 lhs, const Vector2& rhs)
	{
		return Vector2(lhs -= rhs);
	}
	inline const Vector2 operator *(const F32& lhs, Vector2 rhs)
	{
		return Vector2(rhs *= lhs);
	}
	inline const Vector2 operator /(const F32& lhs, Vector2 rhs)
	{
		return Vector2(rhs /= lhs);
	}
	inline const Vector2 operator *(Vector2 lhs, const F32& rhs)
	{
		return Vector2(lhs *= rhs);
	}
	inline const Vector2 operator /(Vector2 lhs, const F32& rhs)
	{
		return Vector2(lhs /= rhs);
	}
	inline const Vector2 operator -(const Vector2& other)
	{
		return -1.0f*other;
	}
	#pragma endregion 
}

