#pragma once
#include "Rendering/Geometry.h"
#include "Rendering/Material.h"

namespace LeEK
{
	/**
	Encapsulates geometry and material properties
	for a collection of polygons that use the same material.
	*/
	class Mesh
	{
	private:
		Material mat;
		Geometry geom;
	public:
		inline const Material& GetMaterial() const { return mat; }
		inline const Geometry& GetGeometry() const { return geom; }
		inline Material& GetMaterial() { return mat; }
		inline Geometry& GetGeometry() { return geom; }
		Mesh(const Material& matParam, const Geometry& geomParam) : mat(matParam), geom(geomParam) {}
		Mesh() : mat(), geom() {}
		~Mesh(void);
	};
}
