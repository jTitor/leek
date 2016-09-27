#pragma once
#include "Datatypes.h"
#include "Math/Plane.h"
#include "Math/Matrix4x4.h"
#include "Bounds.h"

namespace LeEK
{
	/**
	A truncated pyramid. Usually represents the viewing bounds of a camera.
	*/
	class Frustum : public Bounds
	{
	public:
		static const U32 NUM_PLANES = 6;
		static const U32 NUM_POINTS = 8;
	private:
		enum planeIndices
		{
			P_NEAR,
			P_FAR,
			P_LEFT,
			P_RIGHT,
			P_TOP,
			P_BOTTOM
		};
		//current implementation is 6 planes, facing inward into the view space
		Plane planes[NUM_PLANES];
		Vector3 points[NUM_POINTS];

		void initPlanes(const Plane& pLeft, const Plane& pRight,
					const Plane& pTop, const Plane& pBottom,
					const Plane& pNear, const Plane& pFar);
	public:
		static const Frustum Default;
		/**
		Generates a frustrum from an arbitrary view-projection matrix.
		*/
		static Frustum BuildFromMatrix(const Matrix4x4& viewProjMat);
		/**
		Generates a perspective frustrum.
		*/
		/*
		static Frustum BuildPerspFrustrum(	const Vector3& eyePos, 
											const Vector3& eyeDir, 
											const Vector3& up,
											F32 near, F32 far,
											F32 fov);
											*/
		/**
		Generates an orthogonal frustrum.
		*/
		/*
		static Frustum BuildOrthoFrustrum(	const Vector3& eyePos, 
											const Vector3& eyeDir, 
											const Vector3& up,
											F32 near, F32 far,
											F32 top, F32 bottom);
		*/
		Frustum(const Matrix4x4& viewProjMat = Matrix4x4::Identity);
		Frustum(const Frustum& other);
		~Frustum();

		/**
		Accessors for the planes forming the frustum.
		*/
		const Plane& Near() const { return planes[P_NEAR]; }
		const Plane& Far() const { return planes[P_FAR]; }
		const Plane& Top() const { return planes[P_TOP]; }
		const Plane& Bottom() const { return planes[P_BOTTOM]; }
		const Plane& Left() const { return planes[P_LEFT]; }
		const Plane& Right() const { return planes[P_RIGHT]; }
		Plane GetPlane(U32 planeIdx) const;
		
		//Bounds Interface Implementation

		const BoundsType GetType() const;
		F32 Radius() const;
		U32 GetNumPoints() const;
		Vector3 GetBoundPoint(U32 index) const;
		bool Test(const Bounds& other) const;
		bool TestRay(const Vector3& origin, const Vector3& direction) const;
		bool Contains(const Vector3& point) const;
	};
}
