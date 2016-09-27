#pragma once

#include "Strings/String.h"
#include "Rendering/Geometry.h"
#include "Math/Vector3.h"

namespace LeEK
{
	//forward decs
	class Font;

	struct TextVertex
	{
	public:
		Vector3 Position;
		Vector2 TexCoord;
		TextVertex()
		{
			Position = Vector3::Zero;
			TexCoord = Vector2::Zero;
		}
	};

	//class for storing text geometry.
	class TextGeom : public GeomBase<TextVertex, U32>
	{
	public:
		TextGeom() {}
		~TextGeom() {}
	};

	class Text
	{
	private:
		TextGeom geom;
		const Font* font;
		String str;
		//properties?
		bool geomReady;

	public:
		Text(void);
		~Text(void);

		//Returns true if the text geometry doesn't need to be recalculated.
		//In general, the geometry needs to be recalculated whenever the characters or the font change;
		//It's a good idea to keep static text in separate objects to reduce the need for rebuilding.
		bool GeometryReady() const { return geomReady; }

		const TextGeom& GetGeometry() const { return geom; }
		TextGeom& GetGeometry() { return geom; }

		const Font& GetFont()
		const {
			L_ASSERT(font && "Uninitialized font!");
			return *font;
		}
		void SetFont(const Font& val);

		const String& GetString() const { return str; }
		void SetString(const String& val);

		//Generates the geometry needed to render the text.
		void RebuildGeometry();
	};
}