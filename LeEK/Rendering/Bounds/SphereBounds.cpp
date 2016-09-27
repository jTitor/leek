#include "SphereBounds.h"

using namespace LeEK;

SphereBounds::SphereBounds(const Vector3& pCenter, F32 pRadius) : Bounds(pCenter)
{
	radius = pRadius;
}

SphereBounds::~SphereBounds(void)
{
}

F32 SphereBounds::Radius() const
{
	return radius;
}

void SphereBounds::SetRadius(F32 newVal)
{
	radius = Math::Max(newVal, 0.0f);
}

void SphereBounds::SetCenter(const Vector3& newVec)
{
	center = newVec;
}

bool SphereBounds::Test(const Bounds& other) const
{
	if(other.GetType() == GetType())
	{
		if(	Vector3::DistanceSquared(other.Center(), Center()) <= 
			Math::Pow(other.Radius() + Radius(), 2))
		{
			return true;
		}
		return false;
	}
	//All other bounding volumes are more complex; use their intersection test to determine
	//intersection
	return other.Test(*this);
}

bool SphereBounds::TestRay(const Vector3& origin, const Vector3& direction) const
{
	//TODO
	return false;
}

bool SphereBounds::Contains(const Vector3& point) const
{
	//test against squared radius
	F32 sqrDist = Vector3::DistanceSquared(center, point);
	return sqrDist <= (radius * radius);
}