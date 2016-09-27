#include "AABBBounds.h"

using namespace LeEK;

void AABBBounds::init(const Vector3& pCenter, const Vector3& pDimensions)
{
	center = pCenter;
	halfDist = pDimensions.GetAbs() / 2.0f;
}

AABBBounds::~AABBBounds(void)
{
#ifdef _DEBUG
	center = Vector3(FLT_MAX, FLT_MIN, FLT_MAX);
	halfDist = -1.0f * Vector3::One;
#endif
}

F32 AABBBounds::Radius() const
{
	return halfDist.Length();
}

bool AABBBounds::Test(const Bounds& other) const
{
	//TODO
	if(other.GetType() == GetType())
	{
		const AABBBounds& castOther = (const AABBBounds&)other;

		//If the other's an AABB, use the Separating Axis Theorem.
		//Given an axis, boxes are separated when their projected length in that
		//direction do not intersect.
		//Since these are axis aligned, this can be tested via x, y, and z without projection.
		Vector3 min = GetMin();
		Vector3 max = GetMax();
		Vector3 otherMin = castOther.GetMin();
		Vector3 otherMax = castOther.GetMax();

		if(min.X() > otherMax.X() || otherMin.X() > max.X())
		{
			return false;
		}
		if(min.Y() > otherMax.Y() || otherMin.Y() > max.Y())
		{
			return false;
		}
		if(min.Z() > otherMax.Z() || otherMin.Z() > max.Z())
		{
			return false;
		}

		//separated on all axis
		return true;
	}
	//Otherwise, test the other object as if it were a sphere
	else
	{
		Vector3 min = GetMin();
		Vector3 max = GetMax();
		const F32* minArray = min.ToFloatArray();
		const F32* maxArray = max.ToFloatArray();

		F32 distSqr = 0;
		F32 displacement = 0;
		const F32* sphereCenter = other.Center().ToFloatArray();

		//Find the closest point on the box to the sphere.
		//We can do this by clamping the radius of the sphere
		//between the minima and maxima of the AABB.

		for(int i = 0; i < 3; ++i)
		{
			if(sphereCenter[i] < minArray[i])
			{
				displacement = sphereCenter[i] - minArray[i];
				distSqr += Math::Pow(displacement, 2);
			}
			else if(sphereCenter[i] > maxArray[i])
			{
				displacement = sphereCenter[i] - maxArray[i];
				distSqr += Math::Pow(displacement, 2);
			}
		}

		return distSqr <= Math::Pow(other.Radius(), 2);
	}
	return false;
}

bool AABBBounds::TestRay(const Vector3& origin, const Vector3& direction) const
{
	//TODO
	return false;
}

bool AABBBounds::Contains(const Vector3& point) const
{
	//TODO
	return false;
}

F32 AABBBounds::GetWidth() const
{
	return halfDist.X() * 2.0f;
}

void AABBBounds::SetWidth(F32 val)
{
	halfDist.SetX(Math::Max(val / 2.0f, 0.0f));
}

F32 AABBBounds::GetHeight() const
{
	return halfDist.Y() * 2.0f;
}

void AABBBounds::SetHeight(F32 val)
{
	halfDist.SetY(Math::Max(val / 2.0f, 0.0f));
}

F32 AABBBounds::GetDepth() const
{
	return halfDist.Z() * 2.0f;
}

void AABBBounds::SetDepth(F32 val)
{
	halfDist.SetZ(Math::Max(val / 2.0f, 0.0f));
}

void AABBBounds::SetCenter(const Vector3& val)
{
	center = val;
}

Vector3 AABBBounds::GetMin() const
{
	return center - halfDist;
}

Vector3 AABBBounds::GetMax() const
{
	return center + halfDist;
}

Vector3 genDispFactors(int pointIndex)
{
	//since there's 8 points, each
	//bit corresponds to a direction to displace in.
	//multiply by 2 and subtract 1 to convert to [1,-1].
	int x = (2*(pointIndex & (1 << 0))) - 1;
	int y = (2*(pointIndex & (1 << 1))) - 1;
	int z = (2*(pointIndex & (1 << 2))) - 1;
	return Vector3(x,y,z);
}

Vector3 AABBBounds::GetBoundPoint(U32 pointIndex) const
{
	if(pointIndex >= GetNumPoints())
	{
		return Vector3::Zero;
	}

	//generate displacement factors
	Vector3 dispFactor = genDispFactors((int)pointIndex);
	//then multiply by half distances and add to center
	return center + halfDist.ComponentwiseProduct(dispFactor);
}

const Vector3& AABBBounds::GetHalfDimensions() const
{
	return halfDist;
}

void AABBBounds::SetHalfDimensions(const Vector3& val)
{
	if(Math::ApproxEqual(val.LengthSquared(), 0.0f))
	{
		return;
	}
	halfDist = val.GetAbs();
}