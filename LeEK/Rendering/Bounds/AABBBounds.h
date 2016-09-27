#pragma once
#include "Bounds.h"

namespace LeEK
{
	class AABBBounds : public Bounds
	{
	private:
		Vector3 halfDist;

		void init(const Vector3& pCenter, const Vector3& pDimensions);
	public:
		static AABBBounds FromSphere(const Vector3& center, float radius);

		AABBBounds(const Vector3& pCenter = Vector3::Zero, const Vector3& pDimensions = Vector3::Zero) :
			Bounds(pCenter)
		{
			init(pCenter, pDimensions);
		}
		AABBBounds(const Vector3& pCenter, F32 pWidth, F32 pHeight, F32 pDepth) :
			Bounds(pCenter)
		{
			init(pCenter, Vector3(pWidth, pHeight, pDepth));
		}
		/**
		Makes a cubic AABB of the given dimension.
		*/
		AABBBounds(const Vector3& pCenter, F32 pSize) :
			Bounds(pCenter)
		{
			init(pCenter, Vector3(pSize, pSize, pSize));
		}
		~AABBBounds(void);

		//Interface implementation
		const BoundsType GetType() const { return BND_AABB; }

		F32 Radius() const;

		U32 GetNumPoints() const { return 8; }
		Vector3 GetBoundPoint(U32 pointIndex) const;

		bool Test(const Bounds& other) const;
		//TODO
		bool TestRay(const Vector3& origin, const Vector3& direction) const;
		//TODO
		bool Contains(const Vector3& point) const;
		bool Contains(const AABBBounds& otherAABB) const;

		F32 GetWidth() const;
		void SetWidth(F32 val);
		F32 GetHeight() const;
		void SetHeight(F32 val);
		F32 GetDepth() const;
		void SetDepth(F32 val);
		void SetCenter(const Vector3& val);

		Vector3 GetMin() const;
		Vector3 GetMax() const;

		const Vector3& GetHalfDimensions() const;
		void SetHalfDimensions(const Vector3& val);
	};
}
