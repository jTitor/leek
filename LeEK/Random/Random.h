#pragma once
#include "Datatypes.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"

namespace LeEK
{
	namespace Random
	{
		void ResetToSeed(U32* seedValBuf, U32 numSeedElem);
		I32 InRange(I32 min, I32 max);
		F32 InRange(F32 min, F32 max);
		Vector3 InSpace(Vector3 min, Vector3 max);
		Vector3 InCube(F32 cubeMin, F32 cubeMax);
		Vector3 InCube(F32 cubeSideLen);
		Quaternion InEulerRange(Vector3 min, Vector3 max);
		Quaternion InCubicEulerRange(F32 minRad = 0, F32 maxRad = Math::TWO_PI);
		//add some convenience functions maybe
		//InNormalRange? InUnitCircle? InUnitSphere?
	}
}
