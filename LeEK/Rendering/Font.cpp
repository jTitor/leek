#include "Font.h"
#include "Logging/Log.h"
#include "Memory/Allocator.h"
#include "Constants/AllocTypes.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftmodapi.h>
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "DataStructures/BinaryTree.h"
#include "DataStructures/STLContainers.h"

using namespace LeEK;
	
FT_MemoryRec_ memManager;
FT_Library freeType;
bool ftReady = false;

#pragma region Library Handlers
void* FTAlloc(FT_Memory mem, long size)
{
	return Allocator::_CustomMalloc(size, FREETYPE_ALLOC, "FreeTypeAlloc", "Font.cpp", __LINE__);
}

void FTFree(FT_Memory mem, void* ptr)
{
	Allocator::_CustomFree(ptr);
}

void* FTRealloc(FT_Memory mem, long curSize, long newSize, void* ptr)
{
	return Allocator::_CustomRealloc(ptr, newSize, "Font.cpp", __LINE__);
}

bool initFreeType()
{
	//TODO: okay, how do we shut this down?
	memset(&memManager, 0, sizeof(memManager));
	memManager.user = NULL;
	memManager.alloc = FTAlloc;
	memManager.free = FTFree;
	memManager.realloc = FTRealloc;

	//TODO: use engine's memory manager
	FT_Error err = FT_Init_FreeType(&freeType);//FT_New_Library(&memManager, &freeType);
	if(err)
	{
		return false;
	}
	//FT_Add_Default_Modules(freeType);

	return true;
}

void shutdownFreeType()
{
	//FT_Done_Library(freeType);
}
#pragma endregion

typedef std::pair<GlyphElem, GlyphInfo&> GlyphPair;

//dummy vars for references
GlyphInfo invalidInfo;

struct GlyphData
{
public:
	GlyphInfo* Info;
	unsigned char* Bitmap;

	GlyphData(GlyphInfo& info, unsigned char* bitmap) :
		Info(&info), Bitmap(bitmap) {}
	GlyphData() :
		Info(NULL), Bitmap(NULL) {}
	GlyphData(const GlyphData& copy) : 
		Info(copy.Info), Bitmap(copy.Bitmap) {}

	GlyphData& operator= (const GlyphData& copy)
	{
		if(this == &copy)
		{
			return *this;
		}
		//assign stuff
		Info = copy.Info;
		Bitmap = copy.Bitmap;
		return *this;
	}
};

GlyphData invalidGData = GlyphData();

typedef Vector<GlyphData> GlyphDataArray;

//Gets the larger side of a glyph's bounding box.
U32 maxSide(const GlyphInfo* gi)
{
	return Math::Max(gi->Dimensions.Width(), gi->Dimensions.Height());
}

U32 area(const GlyphInfo* gi)
{
	return gi->Dimensions.Width() * gi->Dimensions.Height();
}

//Sorts a list of glyphs by desending maximum side.
bool reverseSort(const GlyphData& left, const GlyphData& right)
{
	return maxSide(left.Info) > maxSide(right.Info);
}

//node used for packing glyphs into a texture.
class GlyphNode : public BTreeNode
{
private:
	IntRectangle texRegion;
	GlyphData& gData;
	bool inUse;
public:
	GlyphNode(GlyphNode* parent, const IntRectangle& texRegionParam) : 
		texRegion(texRegionParam), gData(invalidGData), inUse(false)
	{
		Parent = parent;
	}
	~GlyphNode() {}

	//Returns the region used by the texture in PIXELS.
	inline const IntRectangle& GetTexRegion() const { return texRegion; }

	/*inline const GlyphElem GetGlyph() const { return glyph; }
	void SetGlyph(GlyphElem val) { glyph = val; }

	inline char* GetBitmap() const { return bitmap; }
	void SetBitmap(char* ptr) { bitmap = ptr; }*/

	const GlyphData& GetGlyphData() const { return gData; }
	void SetGlyphData(const GlyphData& val) { gData = val; }

	inline bool InUse() const { return inUse; }
	void SetInUse(bool val) { inUse = val; }

	inline GlyphNode* GetParent() const { return (GlyphNode*)Parent; }
	void SetParent(GlyphNode* val) { Parent = val; }

	inline GlyphNode* GetRight() const { return (GlyphNode*)Right; }
	void SetRight(GlyphNode* val) { Right = val; }

	inline GlyphNode* GetDown() const { return (GlyphNode*)Left; }
	void SetDown(GlyphNode* val) { Left = val; }
};

GlyphNode* newGlyphNode(GlyphNode* parent, const IntRectangle& texRegion)
{
	return CustomNew<GlyphNode>(FONT_ALLOC, "FontAlloc", parent, texRegion);
}

GlyphNode* findFreeNode(GlyphNode* root, F32 glyphWidth, F32 glyphHeight)
{
	//terminating condition
	L_ASSERT(root && "Passing null node!");

	const IntRectangle& texRegion = root->GetTexRegion();
	if(root->InUse())
	{
		GlyphNode* result = NULL;
		//check children
		if(root->GetRight())
		{
			result = findFreeNode(root->GetRight(), glyphWidth, glyphHeight);
		}
		if(!result && root->GetDown())
		{
			result = findFreeNode(root->GetDown(), glyphWidth, glyphHeight);
		}
		return result;
	}
	//otherwise, check this node specifically
	else if(glyphWidth <= texRegion.Width() && glyphHeight <= texRegion.Height())//texRegion.Width() >= glyphWidth && texRegion.Height() >= glyphHeight)
	{
		return root;
	}
	//nothing fits; return null
	return NULL;
}

GlyphNode* splitNode(GlyphNode* node, F32 glyphWidth, F32 glyphHeight)
{
	//first, note this node's used
	node->SetInUse(true);
	const IntRectangle& texRegion = node->GetTexRegion();
	//now generate subnodes
	L_ASSERT(texRegion.Width() >= glyphWidth);
	IntRectangle rightRegion(	texRegion.MinX() + glyphWidth, texRegion.MinY(), 
							texRegion.Width() - glyphWidth, glyphHeight);
	GlyphNode* right = newGlyphNode(node, rightRegion);
	node->SetRight(right);

	L_ASSERT(texRegion.Height() >= glyphHeight);
	IntRectangle downRegion(	texRegion.MinX(), texRegion.MinY() + glyphHeight,
							texRegion.Width(), texRegion.Height() - glyphHeight);
	GlyphNode* down = newGlyphNode(node, downRegion);
	node->SetDown(down);
	return node;
}

bool renderGlyph(FT_Face face, GlyphElem glyph)
{
	//render nothing if character isn't in font
	L_ASSERT(face->charmap);
	U32 glyphInd = glyph;
	FT_UInt glInd = FT_Get_Char_Index(face, glyphInd);
	FT_Error err = FT_Load_Glyph(face, glInd, FT_LOAD_RENDER);
	if(err)
	{
		LogE(String("Couldn't load character ") + glyph + "!");
		return false;
	}
	if(glInd == 0)
	{
		return false;
	}
	return true;
}

bool initGlyphInfo(FT_Face face, GlyphElem glyph, GlyphInfo& info)
{
	if(!renderGlyph(face, glyph))
	{
		//set a null glyph
		info.Glyph = glyph;
		FT_GlyphSlot gs = face->glyph;
		info.Advance = Vector2(gs->advance.x / 64, gs->advance.y / 64);
		info.Dimensions = IntRectangle(	gs->bitmap_left, 
										gs->bitmap_top,
										gs->bitmap.width,
										gs->bitmap.rows);
		info.Descend = Vector2::Zero;
		return true;
	}
	FT_GlyphSlot gs = face->glyph;
	FT_Glyph_Metrics gm = gs->metrics;
	info.Glyph = glyph;
	//advances are listed in 1/64 of pixels,
	//so convert back to pixels
	info.Advance = Vector2(gs->advance.x / 64, gs->advance.y / 64);
	info.Dimensions = IntRectangle(gs->bitmap_left, gs->bitmap_top,	//set top position
								gs->bitmap.width,	//set width
								gs->bitmap.rows);	//set height
	info.Descend = Vector2(gs->bitmap.width - (gm.vertBearingX / 64), gs->bitmap.rows - (gm.horiBearingY / 64));
	return true;
}

//Builds a packed texture atlas for a given set of glyphs.
//Also assigns texture coordinates to all glyph units.
bool buildGlyphTextureAndTexCoords(FT_Face face, GfxWrapperHandle gfx, const Texture2D& tex, U32 paddingLen, U32 texWidth, U32 texHeight, GlyphDataArray::iterator glyphDataStart, GlyphDataArray::iterator glyphDataEnd)
{
	//setup root node for texture
	GlyphNode* const root = newGlyphNode(	NULL,									//root, so no parent
									IntRectangle(0, 0, texWidth, texHeight));	//encloses entire texture
	if(!root)
	{
		LogE("Couldn't build glyph packing tree!");
		return false;
	}

	//now iterate through the list
	for(GlyphDataArray::iterator it = glyphDataStart; it != glyphDataEnd; ++it)
	{
		GlyphInfo* gi = it->Info;
		GlyphElem glyph = gi->Glyph;

		//render the text
		if(!initGlyphInfo(face, glyph, *gi))//renderGlyph(face, glyph))
		{
			return false;
		}
		FT_GlyphSlot gSlot = face->glyph;
		FT_Bitmap gBmap = face->glyph->bitmap;
		char* bitmap = (char*)gBmap.buffer;
		
		F32 gWidth = gi->Dimensions.Width();
		F32 gHeight = gi->Dimensions.Height();

		//need to include upper and lower padding
		F32 paddedWidth = gWidth + (2 * paddingLen);
		F32 paddedHeight = gHeight + (2 * paddingLen);

		//get a node
		GlyphNode* glyphNode = findFreeNode(root, paddedWidth, paddedHeight);
		//if we have one, great; set, split and continue.
		//there should be free nodes in the tree now
		if(glyphNode)
		{
			glyphNode->SetGlyphData(*it);
			//glyphNode->SetBitmap((char*)it->Bitmap);
			splitNode(glyphNode, paddedWidth, paddedHeight);
			
			//now we can compute the texture coordinates.
			//remember to normalize the dimensions!
			const IntRectangle& pixCoords = glyphNode->GetTexRegion();
			L_ASSERT(gi->Glyph == glyphNode->GetGlyphData().Info->Glyph);
			gi->TexCoords = LeEK::Rectangle(	(((F32)pixCoords.MinX() + paddingLen) / texWidth), 
										1.0f - ((F32)(pixCoords.MinY() + paddingLen + gHeight) / texHeight), 
										(gWidth / texWidth), 
										(gHeight / texHeight));

			//advances are listed in 1/64 of pixels
			//gi->Advance.SetX(gSlot->advance.x / 64);
			//gi->Advance.SetY(gSlot->advance.y / 64);

			//and write to the texture
			//we need to use padding, so write slightly offset
			if(!gfx->FillTextureSection(tex, pixCoords.MinX() + paddingLen, pixCoords.MinY() + paddingLen, 
										gWidth, gHeight,
										bitmap))
			{
				LogE(String("Couldn't write glyph \'") + glyph + "\' to texture!");
				return false;
			}
		}
		//otherwise, this node doesn't fit in the texture at all...
		//we're boned, cleanup and quit
		else
		{
			LogE("Couldn't pack font glyphs into texture!");
			DeleteBTree(root);
			return false;
		}
	}
	//Tree should be populated, note success
	//Don't forget to get rid of the packing tree!
	DeleteBTree(root);
	return true;
}

GlyphNode* findGlyphNode(GlyphNode* root, const GlyphData& elem)
{
	//quit if this is the node
	if(root->InUse() && root->GetGlyphData().Info->Glyph == elem.Info->Glyph)
	{
		return root;
	}
	//otherwise, search the children
	else
	{
		GlyphNode* result = findGlyphNode(root->GetRight(), elem);
		if(!result)
		{
			result = findGlyphNode(root->GetDown(), elem);
		}
		return result;
	}
	//and return null otherwise
	return NULL;
}

Font::Font(void) : textureHandle(0), lineHeight(0)
{
}

Font::~Font(void)
{
}

bool Font::GenerateFromMemory(const char* fontFile, GfxWrapperHandle gfx, U32 fileSize, U32 fontSizePixels, U32 maxTexSize)
{
	if(!ftReady)
	{
		if(!initFreeType())
		{
			LogE("Failed to initialize FreeType!");
			return false;
		}
		ftReady = true;
	}

	//load the font now, default face
	FT_Face face;
	FT_Error err = FT_New_Memory_Face(freeType, (const FT_Byte*)(const void*)fontFile, fileSize - 1, 0, &face);
	if(err)
	{
		LogE("Failed to load font!");
		return false;
	}

	//setup desired char size
	err = FT_Set_Pixel_Sizes(face, 0, fontSizePixels);
	if(err)
	{
		LogE("Failed to set font size!");
		return false;
	}

	//now we can set general data
	lineHeight = fontSizePixels;//(F32)(face->height * 4) / 64;

	GlyphDataArray glyphData;
	
	//Now render fonts and build their info structs.
	//Iterate through the ASCII set;
	//we only need to render the visible set, 33 on.
	//Space (32) is special; we don't draw it, it just moves the cursor.
	initGlyphInfo(face, ' ', glyphs[' ']);

	//Do not calculate a region for invisible characters!
	const U32 CHAR_START = 0;
	const U32 CHAR_END = NUM_GLYPHS;
	glyphData.reserve(CHAR_END - CHAR_START);
	for(U32 i = CHAR_START; i < CHAR_END; ++i)
	{
		GlyphInfo& info = glyphs[i];
		if(!initGlyphInfo(face, i, info))
		{
			return false;
		}
		FT_GlyphSlot gs = face->glyph;
		//push this into the pair list if it's a visible character
		//lineHeight = Math::Max(lineHeight, (F32)info.Dimensions.Height());
		if(info.Dimensions.Area() > 0)
		{
			glyphData.push_back(GlyphData(info, gs->bitmap.buffer));
		}
	}
	//sort the rectangles, and pack them.
	std::sort(glyphData.begin(), glyphData.end() - 1, reverseSort);
	
	//and try to build the atlas now
	//create an empty texture
	const U32 MAX_TEX_SIZE = Math::NearestPowOf2(maxTexSize);
	//start from an estimated required size; might not fit exactly, but it often works
	U32 texSize = Math::NearestPowOf2(fontSizePixels * 8);
	while(texSize <= MAX_TEX_SIZE)
	{
		Texture2D tex = gfx->GenerateBlankTexture(texSize, texSize, Texture2D::BYTE);
		if(!tex.TextureBufferHandle)
		{
			LogE("Couldn't build texture for glyph atlas!");
			return false;
		}

		U32 paddingLen = 1;

		//try building the texture
		if(!buildGlyphTextureAndTexCoords(face, gfx, tex, paddingLen, texSize, texSize, glyphData.begin(), glyphData.end()))
		{
			U32 oldTexSize = texSize;
			//double the texture size
			texSize <<= 1;
			LogW(String("Couldn't build glyph atlas at size ") + oldTexSize + ", rebuilding at size " + texSize);
			//dispose of the old texture
			gfx->ShutdownTexture(tex);
		}
		else
		{
			//We succeeded, save the texture file.
			//All other info was initialized by the successful buildGlyphTexture call.
			textureHandle = tex.TextureBufferHandle;
			return true;
		}
	}

	LogE("Couldn't pack texture for glyph atlas!");
	return false;
}

const GlyphInfo& Font::GetGlyphInfo(GlyphElem glyph) const
{
	//we're dumb and store all of the ASCII characters, including INVISIBLE ones,
	//so no transforms are needed
	if(glyph > NUM_GLYPHS)
	{
		return glyphs[0];
	}
	return glyphs[glyph];
}
