#pragma once
#include <memory>
#include "Datatypes.h"
#include "Rendering/Color.h"

namespace LeEK
{
	//Metadata for textures.
	//These aren't a part of Texture 
	//because they're not inherent properties of texture data.
	namespace TextureMeta
	{
		enum MapType { DIFFUSE, SPECULAR, BUMP, NORMAL, GLOW, ENVIRONMENT };
	}

	struct Texture2D
	{
	public:
		//the raw data
		char* Data;
		
		enum PixelType { RGBA8, RGB8, BGRA8, RGB565, BYTE, PIXTYPE_LEN };
		enum CompressionType { NONE, DXT1, /*CRUNCH,*/ COMPTYPE_LEN };

		//general info about texture.
		PixelType PixType;
		CompressionType CompType;
		U32 TextureBufferHandle;
		U32 Width, Height;
		U8 BitDepth;
		bool HasMipMap;

		Texture2D()
		{
			memset(this, 0, sizeof(Texture2D));
		}

		static Texture2D BuildSolidRGBA8Tex(U32 width, U32 height, const Color& color);
	};

	/*
	const char* GetPixelTypeName(Texture2D::PixelType type);
	const char* GetCompressionTypeName(Texture2D::CompressionType type);
	*/
}
