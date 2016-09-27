#pragma once
#include "Math/Vector3.h"
#include "Rendering/Color.h"
#include "ResourceManagement/Resource.h"

namespace LeEK
{
	struct Material
	{
	public://private:
		Color DiffuseColor, SpecularColor, EmissiveColor;
		ResGUID DiffuseTexGUID, SpecularTexGUID, EmissiveTexGUID;
		//optional stuff.
		ResGUID NormalMapGUID;
		Material() :	DiffuseColor(), SpecularColor(), EmissiveColor(), 
						DiffuseTexGUID(), SpecularTexGUID(), EmissiveTexGUID(),
						NormalMapGUID() {}
	/*public:
		//properties
		inline const Color& DiffuseColor() const { return diffuseCol; }
		inline const Color& SpecularColor() const { return specCol; }
		inline const Color& AmbientColor() const { return ambCol; }
		inline const Color& EmissiveColor() const { return emitCol; }
		inline const ResGUID& DiffuseTex() const { return diffuseTex; }
		inline const ResGUID& SpecularTex() const { return specTex; }
		inline const ResGUID& AmbientTex() const { return ambTex; }
		inline const ResGUID& EmissiveTex() const { return emitTex; }
		void SetDiffuseColor(const Color& color) { diffuseCol = color; }
		void SetSpecularColor(const Color& color) { specCol = color; }
		void SetAmbientColor(const Color& color) { ambCol = color; }
		void SetEmissiveColor(const Color& color) { emitCol = color; }
		void SetDiffuseTex(const ResGUID& res) { diffuseTex = res; }
		void SetSpecularTex(const ResGUID& res) { specTex = res; }
		void SetAmbientTex(const ResGUID& res) { ambTex = res; }
		void SetEmissiveTex(const ResGUID& res) { emitTex = res; }*/
	};
}