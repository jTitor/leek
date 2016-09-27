#pragma once
#include "Datatypes.h"
#include "Math/Vector3.h"

namespace LeEK
{
	class Bounds
	{
	public:
		enum BoundsType { BND_SPHERE, BND_AABB, BND_OBB, BND_FRUSTUM, BND_COUNT };
	protected:
		Vector3 center;
		/**
		A test that can be run between any two bounds;
		currently is a sphere test. Not necessarily accurate,
		but allows collisions to be tested if you don't otherwise
		have an implementation for a specific type.
		*/
		bool defaultTest(const Bounds& other) const;
	public:
		Bounds(const Vector3& pCenter = Vector3::Zero);
		virtual ~Bounds(void);

		virtual const BoundsType GetType() const = 0;

		const Vector3& Center() const;
		virtual F32 Radius() const = 0;

		virtual U32 GetNumPoints() const = 0;
		virtual Vector3 GetBoundPoint(U32 index) const = 0;

		/**
		Tests if the volume has collided with another volume.
		*/
		virtual bool Test(const Bounds& other) const = 0;
		virtual bool TestRay(const Vector3& origin, const Vector3& direction) const = 0;
		/**
		Determines if a single point is inside the volume.
		*/
		virtual bool Contains(const Vector3& point) const = 0;
	};
}