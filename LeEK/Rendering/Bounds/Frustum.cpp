#include "Frustum.h"
#include "AABBBounds.h"

using namespace LeEK;

const Frustum Frustum::Default(Matrix4x4::Identity);

Matrix4x4 invertProjMat(const Matrix4x4& pMat)
{
	return pMat.FindInverse();
}

Vector3 genNDCPoint(int pointIndex)
{
	//since there's 8 points, each
	//bit corresponds to a direction to displace in.
	//multiply by 2 and subtract 1 to convert to [1,-1].
	int x = (2*((pointIndex & (1 << 0)) >> 0)) - 1;
	int y = (2*((pointIndex & (1 << 1)) >> 1)) - 1;
	int z = (2*((pointIndex & (1 << 2)) >> 2)) - 1;
	return Vector3(x,y,z);
}

void Frustum::initPlanes(	const Plane& pLeft, const Plane& pRight,
					const Plane& pTop, const Plane& pBottom,
					const Plane& pNear, const Plane& pFar)
{
	planes[P_LEFT] = pLeft;
	planes[P_RIGHT] = pRight;
	planes[P_TOP] = pTop;
	planes[P_BOTTOM] = pBottom;
	planes[P_NEAR] = pNear;
	planes[P_FAR] = pFar;
}

Frustum::Frustum(const Matrix4x4& viewProjMat)
{
	//http://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/
	//simple to extract the planes, they're column-column differences
	Vector4 vNear, vFar, vLeft, vRight, vTop, vBottom;

	/*
	vLeft = Vector4(viewProjMat.GetRow(0) + viewProjMat.GetRow(3));
	vRight = Vector4((-viewProjMat.GetRow(0)) + viewProjMat.GetRow(3));
	vBottom = Vector4(viewProjMat.GetRow(1) + viewProjMat.GetRow(3));
	vTop = Vector4((-viewProjMat.GetRow(1)) + viewProjMat.GetRow(3));
	vNear = Vector4(viewProjMat.GetRow(2) + viewProjMat.GetRow(3));
	vFar = Vector4((-viewProjMat.GetRow(2)) + viewProjMat.GetRow(3));
	*/

	vNear = Vector4(viewProjMat.GetRow(2) + viewProjMat.GetRow(3));
	vFar = Vector4((-viewProjMat.GetRow(2)) + viewProjMat.GetRow(3));
	vBottom = Vector4(viewProjMat.GetRow(1) + viewProjMat.GetRow(3));
	vTop = Vector4((-viewProjMat.GetRow(1)) + viewProjMat.GetRow(3));
	vLeft = Vector4(viewProjMat.GetRow(0) + viewProjMat.GetRow(3));
	vRight = Vector4((-viewProjMat.GetRow(0)) + viewProjMat.GetRow(3));

	//normalize them (literally, normalize the coefficient vector)
	//to use with sphere distance comparisons
	/*
	vLeft.Normalize();
	vRight.Normalize();
	vBottom.Normalize();
	vTop.Normalize();
	vNear.Normalize();
	vFar.Normalize();
	*/

	initPlanes(	Plane(vLeft), Plane(vRight),
					Plane(vTop), Plane(vBottom),
					Plane(vNear), Plane(vFar));

	//now we need to get the points -
	//invert the projmat and multiply a symmetric cube
	//spanning [-1,1] in each direction
	//by that inverse
	Matrix4x4 invMat = invertProjMat(viewProjMat);

	for(int i = 0; i < NUM_POINTS; ++i)
	{
		points[i] = invMat.MultiplyPoint(genNDCPoint(i));
	}
}

Frustum::Frustum(const Frustum& other)
{
	//copy data
	for(int i = 0; i < NUM_PLANES; ++i)
	{
		planes[i] = other.planes[i];
	}

	for(int i = 0; i < NUM_POINTS; ++i)
	{
		points[i] = other.points[i];
	}
}

Frustum::~Frustum()
{
}

Plane Frustum::GetPlane(U32 planeIdx) const
{
	if(planeIdx >= NUM_PLANES)
	{
		return Plane();
	}
	return planes[planeIdx];
}

Frustum Frustum::BuildFromMatrix(const Matrix4x4& vpMat)
{
	return Frustum(vpMat);
}

const Bounds::BoundsType Frustum::GetType() const
{
	return BoundsType::BND_FRUSTUM;
}

F32 Frustum::Radius() const
{
	return 0;
}

U32 Frustum::GetNumPoints() const
{
	return NUM_POINTS;
}

Vector3 Frustum::GetBoundPoint(U32 index) const
{
	if(index < NUM_POINTS)
	{
		return points[index];
	}
	return Vector3::Zero;
}

bool Frustum::Test(const Bounds& other) const
{
	if(other.GetType()==BoundsType::BND_AABB)
	{
		const AABBBounds& castOther = (const AABBBounds&)other;
		//int numIn = 0;
		
		//It's an AABB, so we can greatly simplify this.

		for(int i = 0; i < NUM_PLANES; ++i)
		{
			Vector3 plane = planes[i].GetCoefficients().XYZ();
			Vector3 absPlane = plane.GetAbs();

			F32 centerDist = castOther.Center().Dot(plane);
			F32 extentDist = castOther.GetHalfDimensions().Dot(absPlane);

			bool intersects = centerDist + extentDist > -planes[i].GetCoefficients().W();
			bool contains = centerDist - extentDist >= -planes[i].GetCoefficients().W();

			//what matters is if this is even partly in the frustum;
			//if centerDist - extentDist is >= -plane.w, the box is completely in the frustum.
			/*
			if(intersects || contains)
			{
				numIn++;
			}
			*/
			if(!intersects && !contains)
			{
				return false;
			}
		}
		return true;
	}
	//This is for spheres,
	//but as there's no other cases we currently handle,
	//it's the default collision test for this type.
	else //if(other.GetType() == BoundsType::BND_SPHERE)
	{
		int numIn = 0;
		//sort of like the point test, but we also include the sphere radius.
		for(int i = 0; i < NUM_PLANES; ++i)
		{
			F32 dist = planes[i].GetDist(other.Center());
			//check that the sphere is completely outside the plane
			//and that it's not intersecting the plane.
			if(dist < -other.Radius() && Math::Abs(dist) >= other.Radius())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool Frustum::TestRay(const Vector3& origin, const Vector3& direction) const
{
	//TODO
	return false;
}

bool Frustum::Contains(const Vector3& point) const
{
	for(int i = 0; i < NUM_PLANES; ++i)
	{
		if(planes[i].GetSide(point) < 0)
		{
			return false;
		}
	}
	return true;
}