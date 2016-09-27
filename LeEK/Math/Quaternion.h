#pragma once
#include "Datatypes.h"
#include "MathFunctions.h"
#include "Vector3.h"
#include "Matrix4x4.h"

namespace LeEK
{
	//has the same memory space as a Vector4, 
	//but they're incompatible in subtle ways, so can't subclass from Vector4.
	//quickly becomes inaccurate after 2 or 3 concatations! be very careful with compound rotations!
	//Quaternions are left transformation quaternions in this engine!
	class Quaternion
	{
	private:
		F32 x, y, z, w;
	public:
		Quaternion(void);
		Quaternion(F32 x, F32 y, F32 z, F32 w);
		Quaternion(const Vector3& axis, F32 angle);
		~Quaternion(void);

		static const Quaternion Identity;

#pragma region Accessors
		//Don't use direct setters unless you know what you're doing!
		inline const F32 X() const { return x; }
		inline void SetX(F32 val) { x = val; }
		inline const F32 Y() const { return y; }
		inline void SetY(F32 val) { y = val; }
		inline const F32 Z() const { return z; }
		inline void SetZ(F32 val) { z = val; }
		inline const F32 W() const { return w; }
		inline void SetW(F32 val) { w = val; }

		inline Vector3 GetVectorPart() const { return Vector3(x, y, z); }
		inline F32 GetScalarPart() const { return w; }
#pragma endregion

#pragma region Operators
		//We can add and scale quaternions, but of course they won't be unit quaternions anymore
		//make sure to normalize after using these operations!
		//no assignment operator?
		inline Quaternion& operator +=(const Quaternion& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;

			return *this;
		}
		inline Quaternion& operator -=(const Quaternion& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;

			return *this;
		}
		inline Quaternion& operator *=(const Quaternion& rhs)
		{
			//PROFILE("QuatMult");
			w = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
			x = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
			y = w*rhs.y - x*rhs.z + y*rhs.w + z*rhs.x;
			z = w*rhs.z + x*rhs.y - y*rhs.x + z*rhs.w;

			/*Vector3 vRhs = Vector3(rhs.x,rhs.y,rhs.z);
			Vector3 vLhs = Vector3(x,y,z);

			w = rhs.w*w - vRhs.Dot(vLhs);
			Vector3 vFinal = rhs.w*vLhs + w*vRhs + vLhs.Cross(vRhs);
			x = vFinal.X();
			y = vFinal.Y();
			z = vFinal.Z();*/

			Normalize();

			return *this;
		}
		inline Quaternion& operator *=(const F32 rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;

			return *this;
		}
		inline Quaternion& operator /=(const F32 rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			w /= rhs;

			return *this;
		}
		inline Quaternion& operator -()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;

			return *this;
		}
#pragma endregion

		String ToString() const;

		inline F32 Magnitude() const { return Math::Sqrt(MagnitudeSquared()); }
		inline F32 MagnitudeSquared() const
		{
			return x*x + y*y + z*z + w*w;
		}

		inline void Normalize()
		{
			F32 sqrLen = (*this).MagnitudeSquared();
			if (!Math::ApproxEqual(sqrLen, 1.0f))
			{
				(*this).operator/=(Math::Sqrt(sqrLen));
			}
		}
		inline Quaternion GetNormalized() const
		{
			Quaternion result = *this;
			result.Normalize();
			return result;
		}
		inline Quaternion GetInverse() const { return Quaternion(-x, -y, -z, w); }

		Matrix4x4 ToMatrix() const;
		inline Vector4 ToVector4() const { return Vector4(x, y, z, w); }

		//Quaternion ParallelFromQuaternion(const Quaternion& q);?
		//(c.f.)Essential Math For Games p.192

		//with quaternions, the dot product tells us the similarity of their rotations
		//not sure where that's used, but it could be important?
		inline F32 Dot(const Quaternion& rhs)
		{
			return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
		}

#pragma region Builders
		static Quaternion FromMatrix(const Matrix4x4& M);
		static Quaternion FromEulerAngles(F32 xRads, F32 yRads, F32 zRads);

		static Quaternion FromEulerAngles(const Vector3& v) { return FromEulerAngles(v.X(), v.Y(), v.Z()); }
		static inline Quaternion FromVector(const Vector3& v) { return Quaternion(v.X(), v.Y(), v.Z(), 0.0f); }
		static inline Quaternion BuildXRotation(F32 angleRads) { return Quaternion(Vector3(1.0f, 0.0f, 0.0f), angleRads); }
		static inline Quaternion BuildYRotation(F32 angleRads) { return Quaternion(Vector3(0.0f, 1.0f, 0.0f), angleRads); }
		static inline Quaternion BuildZRotation(F32 angleRads) { return Quaternion(Vector3(0.0f, 0.0f, 1.0f), angleRads); }
#pragma endregion

		Vector3 RotateVector(const Vector3& v) const;
	};

	//comparison operators
	inline bool operator==(const Quaternion& lhs, const Quaternion& rhs){ return	Math::ApproxEqual(const_cast<Quaternion&>(lhs).X(), const_cast<Quaternion&>(rhs).X()) &&
																					Math::ApproxEqual(const_cast<Quaternion&>(lhs).Y(), const_cast<Quaternion&>(rhs).Y()) && 
																					Math::ApproxEqual(const_cast<Quaternion&>(lhs).Z(), const_cast<Quaternion&>(rhs).Z()) &&
																					Math::ApproxEqual(const_cast<Quaternion&>(lhs).W(), const_cast<Quaternion&>(rhs).W()); } 
	inline bool operator!=(const Quaternion& lhs, const Quaternion& rhs){ return !operator==(lhs,rhs); }

	//static operators
	inline Quaternion operator +(Quaternion lhs, const Quaternion& rhs)
	{
		lhs += rhs;
		return lhs;
	}
	inline Quaternion operator -(Quaternion lhs, const Quaternion& rhs)
	{
		lhs -= rhs;
		return lhs;
	}
	inline Quaternion operator *(Quaternion lhs, const F32 rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Vector3 operator *(Quaternion lhs, const Vector3& rhs)
	{
		return lhs.RotateVector(rhs);
	}
	inline Quaternion operator *(Quaternion lhs, const Quaternion& rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	inline Quaternion operator /(Quaternion lhs, const F32 rhs)
	{
		lhs /= rhs;
		return lhs;
	}
}

