#pragma once
#include "Datatypes.h"
#include "Math/Vectors.h"
#include "Math/Quaternion.h"

namespace LeEK
{
	/**
	Describes a tranformation on a Vector3 point.
	Currently does NOT support nonuniform scaling.
	*/
	class Transform
	{
	private:
		//note the order - do NOT operate the same way.
		//scale, rotate, translate.
		Quaternion orien;
		Vector3 trans;
		F32 scale;
	public:
		static const Transform Identity;

		Transform(	const Vector3& transP = Vector3::Zero,
					const Quaternion& orienP = Quaternion::Identity, 
					F32 scaleP = 1.0) : trans(transP), orien(orienP), scale(scaleP)
		{
		}
		~Transform(void);
		const Vector3& Position() const { return trans; }
		const Quaternion& Orientation() const { return orien; }
		Vector3& Position() { return trans; }
		Quaternion& Orientation() { return orien; }
		F32 Scale() const { return scale; }

		void SetPosition(const Vector3& newPos)
		{
			trans = newPos;
		}
		void SetOrientation(const Quaternion& newOrien)
		{
			orien = newOrien;
		}
		void Translate(const Vector3& newPos)
		{
			SetPosition(trans + newPos);
		}
		void Rotate(const Quaternion& newOrien)
		{
			SetOrientation(orien * newOrien);
		}

		//concatenation
		Transform& operator*=(const Transform& other);

		Matrix4x4 ToMatrix() const;

		Vector3 TransformVector(const Vector3& v) const;

		//Transform Invert();?

		//brutal, but might want to be able to display this, after all
		Vector3 ToEulerAngles() const;
		String ToString() const;
	};

	inline const Transform operator *(Transform lhs, const Transform& rhs)
	{
		return Transform(lhs *= rhs);
	}
}