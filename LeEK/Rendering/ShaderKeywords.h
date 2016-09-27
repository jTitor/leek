/**
Reserved keywords for any shaders used in the engine.
If you define a keyworded uniform or var with the wrong type,
I'll uh, I dunno, yell at you or something.
I'll figure it out later.
*/
#pragma once

#include "../Datatypes.h"
#include "../Math/Matrix3x3.h"
#include "../Math/Vector3.h"

namespace LeEK
{
	//int keywords

	//float keywords

	//vec2 keywords
	const char* SKEY_TEX_COORD = "lTexCoord";

	//vec3 keywords
	const char* SKEY_COLOR = "lColor";
	//	lighting attributes
	const char* SKEY_LIGHT_COLOR = "lLightColor";
	const char* SKEY_LIGHT_POS = "lLightPos";

	//mat4x4 keywords
	const char* SKEY_WORLD_MAT = "lWorldMat";
	const char* SKEY_VIEW_MAT = "lViewMat";
	const char* SKEY_PROJ_MAT = "lProjMat";

	//texture/sampler keywords
	const char* SKEY_DIFF_TEX = "lDiffTex";
	const char* SKEY_SPEC_TEX = "lSpecTex";
	const char* SKEY_NORM_TEX = "lNormTex";
	const char* SKEY_GLOW_TEX = "lGlowTex";
}