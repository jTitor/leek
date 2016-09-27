#pragma once
#include "Datatypes.h"
#include "Math/Rectangle.h"
#include "ResourceManagement/ResourceManager.h"
#include "DataStructures/BinaryTree.h"
#include "GraphicsWrappers/IGraphicsWrapper.h"

namespace LeEK
{
	//The basic element of a text string.
	//For now, it's ASCII, so each element is a char.
	typedef char GlyphElem;

	struct GlyphInfo
	{
		//distance to the glyph in front of this glyph
		Vector2 Advance;
		//length of the glyph that is below the baseline
		Vector2 Descend;
		IntRectangle Dimensions;
		Rectangle TexCoords;	//normalized tex coords
		GlyphElem Glyph;

		GlyphInfo(const GlyphElem glyph, const Vector2& advance, const IntRectangle& dims, const Rectangle& texCoords) :
			Glyph(glyph), Advance(advance), Descend(Vector2::Zero), Dimensions(dims), TexCoords(texCoords) {}
		GlyphInfo() : Glyph(0), Advance(Vector2::Zero), Descend(Vector2::Zero), Dimensions(), TexCoords() {}
	};

	//Stores all information necessary for a renderer to draw text.
	//Presently only supports ASCII characters.
	class Font
	{
	private:
		static const U32 NUM_GLYPHS = 128;

		U32 textureHandle;
		F32 lineHeight;
		GlyphInfo glyphs[NUM_GLYPHS];

	public:
		Font(void);
		~Font(void);

		bool GenerateFromMemory(const char* fontFile, GfxWrapperHandle gfx,  U32 fileSize, U32 fontSizePixels, U32 maxTexSize);
		bool GenerateFromFile(DataStream* file, GfxWrapperHandle gfx, U32 fontSizePixels, U32 maxTexSize)
		{
			//read into buffer, and generate
			char* buffer = file->ReadAll();
			bool result = GenerateFromMemory(buffer, gfx, file->FileSize(), fontSizePixels, maxTexSize);
			CustomArrayDelete(buffer);
			return result;
		}
		bool GenerateFromResource(ResPtr resource, GfxWrapperHandle gfx, U32 fontSizePixels, U32 maxTexSize)
		{ 
			return GenerateFromMemory(resource->Buffer(), gfx, resource->Size(), fontSizePixels, maxTexSize);
		}

		const GlyphInfo& GetGlyphInfo(GlyphElem glyph) const;
		U32 GetTextureHandle() const { return textureHandle; }

		F32 GetLineHeight() const { return lineHeight; } 
	};
}
