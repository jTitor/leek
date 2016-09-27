#pragma once
#include "Vector2.h"

namespace LeEK
{
	template<typename T>
	class rectangleBase_
	{
	private:
		T minX, minY, maxX, maxY;
	public:
		rectangleBase_(T x, T y, T width, T height) :
			minX(x), minY(y), maxX(x + width), maxY(y + height) {}
		rectangleBase_(Vector2& minParam, Vector2& maxParam) : 
			minX(minParam.X()), minY(minParam.Y()), maxX(maxParam.X()), maxY(maxParam.Y()) {}
		rectangleBase_(void)
		{
			minX = 0;
			minY = 0;
			maxX = 0;
			maxY = 0;
		}
		~rectangleBase_(void) {}
		
		inline T MinX() const { return minX; }
		inline T MinY() const { return minY; }
		inline T MaxX() const { return maxX; }
		inline T MaxY() const { return maxY; }
		inline const Vector2& Min() const { return Vector2(minX, minY); }
		inline const Vector2& Max() const { return Vector2(maxX, maxY); }
		inline void SetMinX(T val) { minX = val; }
		inline void SetMinY(T val) { minY = val; }
		inline void SetMaxX(T val) { maxX = val; }
		inline void SetMaxY(T val) { maxY = val; }
		inline void SetMin(Vector2& val)
		{ 
			minX = val.X();
			minY = val.Y();
		}
		inline void SetMax(Vector2& val)
		{ 
			maxX = val.X();
			maxY = val.Y();
		}

		T Width() const
		{ 
			L_ASSERT(MaxX() >= MinX() && "Malformed rectangle!");
			return MaxX() - MinX();
		}
		T Height() const
		{
			L_ASSERT(MaxY() >= MinY() && "Malformed rectangle!");
			return MaxY() - MinY();
		}
		T Area() const
		{
			return Width() * Height();
		}
		Vector2 Center() const
		{
			return Vector2(MinX() + (Width() / 2.0f), MinY() + (Height() / 2.0f));
		}
	};

	typedef rectangleBase_<F32> Rectangle;
	typedef rectangleBase_<I32> IntRectangle;
}