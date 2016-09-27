#pragma once
#include "Datatypes.h"
#include "Rendering/Texture.h"
#include "Rendering/Shader.h"

namespace LeEK
{
	/**
	* Interface for a graphical effect unit.
	* Composed of layers of pairs of shaders; each layer is called a pass.
	* Each pass can contain multiple textures.
	*/
	class IShaderEffect
	{
	public:
		virtual U32 GetNumPasses() = 0;
		virtual U32 GetNumTextures(U32 pass) = 0;
		virtual const Shader& GetShader(U32 pass) = 0;
		virtual const Texture2D& GetTexture(U32 pass, U32 texIndex) = 0;
	};
}