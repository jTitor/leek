#pragma once
#include "Bounds.h"
namespace LeEK
{
	class SphereBounds : public Bounds
	{
	private:
		F32 radius;
	public:
		SphereBounds(const Vector3& pCenter = Vector3::Zero, F32 pRadius = 0.0f);
		~SphereBounds(void);
		
		//Interface implementation
		const BoundsType GetType() const { return BND_SPHERE; }

		F32 Radius() const;
		void SetRadius(F32 newVal);
		void SetCenter(const Vector3& newVec);

		U32 GetNumPoints() const { return 1; }
		Vector3 GetBoundPoint(U32 index) const { return center; }

		bool Test(const Bounds& other) const;
		//TODO
		bool TestRay(const Vector3& origin, const Vector3& direction) const;
		bool Contains(const Vector3& point) const;
	};
}