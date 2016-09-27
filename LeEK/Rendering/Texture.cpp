#include "Texture.h"
#include "Constants/AllocTypes.h"

using namespace LeEK;

Texture2D Texture2D::BuildSolidRGBA8Tex(U32 width, U32 height, const Color& color)
{
	Texture2D res = Texture2D();
	res.BitDepth = 32;
	res.HasMipMap = false;
	res.PixType = Texture2D::RGBA8;
	res.CompType = Texture2D::NONE;
	res.Width = width;
	res.Height = height;
	int numPxls = width*height;
	int charsPerPxl = (res.BitDepth / 8);
	res.Data = LArrayNew(char, numPxls*charsPerPxl, AllocType::RENDERER_ALLOC, "RendererAlloc");
	//now fill the data with the specified color
	for(int i = 0; i < numPxls; ++i)
	{
		U32 pxlIdx = (i*numPxls);
		res.Data[pxlIdx++] = color.R();
		res.Data[pxlIdx++] = color.G();
		res.Data[pxlIdx++] = color.B();
		res.Data[pxlIdx++] = color.A();
	}

	return res;
}