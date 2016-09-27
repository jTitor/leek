#include "Bounds.h"

using namespace LeEK;

Bounds::Bounds(const Vector3& pCenter)
{
	center = pCenter;
}


Bounds::~Bounds(void)
{
#ifdef _DEBUG
	center = Vector3(FLT_MAX, FLT_MIN, FLT_MAX);
#endif
}

bool Bounds::defaultTest(const Bounds& other) const
{
	//All other bounds provide enough information to do a sphere test
	if(	Vector3::DistanceSquared(other.Center(), Center()) <= 
		Math::Pow(other.Radius() + Radius(), 2))
	{
		return true;
	}
	return false;
}

const Vector3& Bounds::Center() const
{
	return center;
}

/*
Vector3 Bounds::GetBoundPoint(U32 index) const
{
	return center;
}

const Bounds::BoundsType Bounds::GetType() const
{
	//technically true?
	return BND_SPHERE;
}
F32 Bounds::Radius() const
{
	return 0;
}
U32 Bounds::GetNumPoints() const
{
	return 1;
}
bool Bounds::Test(const Bounds& other) const
{
	return defaultTest(other);
}
bool Bounds::TestRay(const Vector3& origin, const Vector3& direction) const
{
	return false;
}
bool Bounds::Contains(const Vector3& point) const
{
	return false;
}*/