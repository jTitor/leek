#include "Text.h"
#include "Constants/AllocTypes.h"
#include "Font.h"

using namespace LeEK;

const U32 MIN_REALLOC_DIFF = 32;

Text::Text(void)
{
	geomReady = false;
	font = NULL;
	geom.Initialize(0, 0, 0, 0);
}


Text::~Text(void)
{
}

void Text::SetFont(const Font& val)
{
	font = &val;
	geomReady = false;
}

void Text::SetString(const String& val)
{ 
	str = val;
	geomReady = false;
}

void Text::RebuildGeometry()
{
	//geometry has changed;
	//iff the string is longer than it was previously, we need to realloc
	int newStrLen = str.length();
	int oldStrLen = geom.VertexCount() / 4;
	//each character needs 4 vertices for a quad
	const U32 newGeomLen = newStrLen * 4;
	//and the entire string is in indexed triangles, so that's 6 * strlen
	//(2 tris per quad, 1 quad per glyph)
	const U32 newIndexLen = newStrLen * 6;
	TextVertex* vertices = HandleMgr::GetPointer<TextVertex>(geom.VertexHandle());
	U32* indices = HandleMgr::GetPointer<U32>(geom.IndexHandle());
	TypedArrayHandle<TextVertex> vertHnd = geom.VertexHandle();
	TypedArrayHandle<U32> indexHnd = geom.IndexHandle();
	//if(newGeomLen > geom.VertexCount() || newIndexLen > geom.IndexCount())
	//{
	//we're allocating new buffers, we can get rid of the old
	//or maybe we can't? This makes the allocator crap itself
	//Appears to be something wrong with allocating the text vertices.
	if(newStrLen > oldStrLen || newStrLen <= oldStrLen / 2)
	{
		vertHnd = geom.VertexHandle();
		indexHnd = geom.IndexHandle();
		geom.Initialize(0, 0, newGeomLen, newIndexLen);
		if(vertHnd.GetHandle() != 0)
		{
			HandleMgr::DeleteArrayHandle(vertHnd);
		}
		if(indexHnd.GetHandle() != 0)
		{
			HandleMgr::DeleteArrayHandle(indexHnd);
		}

		auto oldVert = vertices;
		auto oldInd = indices;
		vertices = CustomArrayNew<TextVertex>(newGeomLen, FONT_ALLOC, "FontAlloc");
		//L_ASSERT(oldVert != vertices);
		indices = CustomArrayNew<U32>(newIndexLen, FONT_ALLOC, "FontAlloc");
		//L_ASSERT(oldInd != indices);

		vertHnd = (TypedArrayHandle<TextVertex>)HandleMgr::RegisterPtr(vertices);
		indexHnd = (TypedArrayHandle<U32>)HandleMgr::RegisterPtr(indices);
	}
		//geom.Initialize(vertHnd, indexHnd, newGeomLen, newIndexLen);
	//}
	//otherwise we can fill the current geometry arrays and reset the data counts
	//keep track of the lower left corner of the current character
	Vector2 cursorPos = Vector2::Zero;
	//iterate through the string:
	U32 strLen = Math::Min(oldStrLen, newStrLen);
	for(U32 i = 0; i < strLen; ++i)//str.length(); i++)
	{
		char glyph = str[i];
		//lookup the character's glyph data
		const GlyphInfo& gInf = font->GetGlyphInfo(glyph);
		//build a quad based on that data
		const U32 vIndexBase = i * 4;
		const F32 scaleFactor = 1.0f / 512;
		const IntRectangle& dims = gInf.Dimensions;
		const Vector2& desc = gInf.Descend;
		
		//if the character is a newline, just move down a section.
		if(glyph == '\n')
		{
			cursorPos.SetX(0);
			cursorPos.SetY(cursorPos.Y() - (font->GetLineHeight() * scaleFactor));
			continue;
		}

		//Need to make a quad with the same bounds as the glyph;
		//However, it also needs to be properly positioned.
		//Characters with bits below the baseline need to be shifted down.
		vertices[vIndexBase].Position		= Vector3(cursorPos.X(), cursorPos.Y() - (desc.Y() * scaleFactor), 0);	//bottom left
		vertices[vIndexBase + 1].Position	= Vector3(	cursorPos.X() + (dims.Width() * scaleFactor),
												cursorPos.Y() - (desc.Y() * scaleFactor),
												0);	//bottom right
		vertices[vIndexBase + 2].Position	= Vector3(	cursorPos.X(),
												cursorPos.Y() + ((dims.Height() - desc.Y()) * scaleFactor),
												0);	//top left
		vertices[vIndexBase + 3].Position	= Vector3(	cursorPos.X() + (dims.Width() * scaleFactor),
												cursorPos.Y() + ((dims.Height() - desc.Y()) * scaleFactor),
												0); //top right
		//set the texture coordinates!
		const Rectangle& texRect = gInf.TexCoords;
		vertices[vIndexBase].TexCoord		= Vector2(texRect.MinX(), texRect.MinY());//, 0);	//bottom left
		vertices[vIndexBase + 1].TexCoord	= Vector2(texRect.MaxX(), texRect.MinY());//, 0);	//bottom right
		vertices[vIndexBase + 2].TexCoord	= Vector2(texRect.MinX(), texRect.MaxY());//, 0); //top left
		vertices[vIndexBase + 3].TexCoord	= Vector2(texRect.MaxX(), texRect.MaxY());//, 0); //top right

		//also build the indices

		const U32 iIndexBase = i * 6;
		indices[iIndexBase] = vIndexBase;
		indices[iIndexBase + 1] = vIndexBase + 1;
		indices[iIndexBase + 2] = vIndexBase + 2;
		indices[iIndexBase + 3]  = vIndexBase + 3;
		indices[iIndexBase + 4] = vIndexBase + 2;
		indices[iIndexBase + 5] = vIndexBase + 1;
		//and move the cursor ahead
		cursorPos += gInf.Advance * scaleFactor;
	}
	//notify geometry of new state
	geom.Initialize(vertHnd, indexHnd, newGeomLen, newIndexLen);
	geomReady = true;
}