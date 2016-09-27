#pragma once
#include "Datatypes.h"
#include "Math/Vector4.h"
#include "Math/Vector4.h"

namespace LeEK
{
	/**
	*	Color object.
	*	Stores color in a RGBA float vector, in the range [0.0f, 1.0f].
	*	Will clamp values outside this range!
	*/
	//TODO
	class Color
	{
	private:
		F32 values[4];//values[0],values[1],values[2],values[3];
		inline void init(F32 r, F32 g, F32 b, F32 a)
		{
			values[0] = Math::Clamp<F32>(r, 0.0f, 1.0f);
			values[1] = Math::Clamp<F32>(g, 0.0f, 1.0f);
			values[2] = Math::Clamp<F32>(b, 0.0f, 1.0f);
			values[3] = Math::Clamp<F32>(a, 0.0f, 1.0f);
		}
	public:
		Color(F32 r, F32 g, F32 b, F32 a)
		{
			init(r, g, b, a);
		}
		Color(F32 r, F32 g, F32 b)
		{
			init(r, g, b, 1.0f);
		}
		Color(Vector4 colorVec)
		{
			init(colorVec.X(), colorVec.Y(), colorVec.Z(), colorVec.W());
		}
		Color(Vector3 colorVec)
		{
			init(colorVec.X(), colorVec.Y(), colorVec.Z(), 1.0f);
		}
		Color(void)
		{
			init(0.0f, 0.0f, 0.0f, 1.0f);
		}

		#pragma region Properties
		inline F32 R() const { return values[0]; }
		inline F32 G() const { return values[1]; }
		inline F32 B() const { return values[2]; }
		inline F32 A() const { return values[3]; }
		Vector3 GetRGB() const { return Vector3(R(), G(), B()); }

		inline void SetR(F32 val) { values[0] = Math::Clamp<F32>(val, 0.0f, 1.0f); }
		inline void SetG(F32 val) { values[1] = Math::Clamp<F32>(val, 0.0f, 1.0f); }
		inline void SetB(F32 val) { values[2] = Math::Clamp<F32>(val, 0.0f, 1.0f); }
		inline void SetA(F32 val) { values[3] = Math::Clamp<F32>(val, 0.0f, 1.0f); }
		#pragma endregion

		inline const F32* ToFloatArray() const { return values; }
		String ToString() const;
	};

	namespace Colors
	{
		static const Color Red(1.0f, 0.0f, 0.0f);
		static const Color Pink(1.0f, 0.5f, 0.5f);
		//eh, why not
		static const Color LtRed(Pink);
		static const Color Green(0.0f, 1.0f, 0.0f);
		static const Color LtGreen(0.5f, 1.0f, 0.5f);
		static const Color Blue(0.0f, 0.0f, 1.0f);
		static const Color LtBlue(0.5f, 0.5f, 1.0f);
		static const Color Magenta(1.0f, 0.0f, 1.0f);
		static const Color LtMagenta(1.0f, 0.5f, 1.0f);
		static const Color Cyan(0.0f, 1.0f, 1.0f);
		static const Color LtCyan(0.5f, 1.0f, 1.0f);
		static const Color Orange(1.0f, 0.5f, 0.0f);
		static const Color Yellow(1.0f, 1.0f, 0.0f);
		static const Color LtYellow(1.0f, 1.0f, 0.5f);
		static const Color Black(0.0f, 0.0f, 0.0f);
		static const Color White(1.0f, 1.0f, 1.0f);
		static const Color Clear(0.0f, 0.0f, 0.0f, 0.0f);

		//value series
		static const Color Gray10Pct(0.1f, 0.1f, 0.1f);
		static const Color Gray20Pct(0.2f, 0.2f, 0.2f);
		static const Color Gray30Pct(0.3f, 0.3f, 0.3f);
		static const Color Gray40Pct(0.4f, 0.4f, 0.4f);
		static const Color Gray50Pct(0.5f, 0.5f, 0.5f);
		static const Color Gray60Pct(0.6f, 0.6f, 0.6f);
		static const Color Gray70Pct(0.7f, 0.7f, 0.7f);
		static const Color Gray80Pct(0.8f, 0.8f, 0.8f);
		static const Color Gray90Pct(0.9f, 0.9f, 0.9f);
	}
}