#pragma once
#include "Vector4.h"

namespace LeEK
{
	/**
	Represents a plane.
	*/
	class Plane
	{
	private:
		Vector4 coeff;
		void init(const Vector4& pCoeff);
	public:
		/**
		Generates a plane given a point on the plane and the plane's normal. Note that the normal <i>must</i> be normalized or the plane will not be at the expected position from the origin!
		@param origin A point on the plane.
		@param normal The plane's normal; specifically, a <i>normalized</i> vector perpendictular to the plane, pointing in the direction of the side that will be the front of the plane.
		*/
		Plane(const Vector3& origin, const Vector3& normal);
		Plane(const Vector4& coeff = Vector4::Zero);
		~Plane(void);
		/**
		Gets the signed distance of the point to the plane.
		*/
		F32 GetDist(const Vector3& point) const;
		/**
		Determines what side a point is on this plane.
		1 indicates that the point is in front of the plane,
		0 that the point is on the plane,
		and -1 indicates that the point is behind the plane.
		*/
		I32 GetSide(const Vector3& point) const;
		/**
		Returns the plane's normal.
		*/
		Vector3 GetNormal() const;
		/**
		Returns the plane's origin: a point on the plane that, combined with <0,0,0>, has the same direction as the plane normal.
		*/
		Vector3 GetOrigin() const;
		void SetNormal(const Vector3& val);
		void SetOrigin(const Vector3& val);

		const Vector4& GetCoefficients() const;
	};
}