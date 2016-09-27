#pragma once
#include "Datatypes.h"
#include "ModelNode.h"
#include "LightNode.h"

namespace LeEK
{
	struct VisibleElement
	{
	public:
		SpatialHnd Spatial;
		Vector<TypedHandle<Shader>> GlobalShaders;

		VisibleElement(SpatialHnd spatial = 0, Vector<TypedHandle<Shader>> globalShaders = Vector<TypedHandle<Shader>>())
		{
			Spatial = spatial;
			GlobalShaders = globalShaders;
		}
	};

	class VisibleSet
	{
	public:
		Vector<VisibleElement> Elements;
		//Light nodes affect all nodes under their parent?
		Vector<LightNode> LightNodes;

		VisibleSet(U32 elemReserve = 1, U32 lightReserve = 1);
		~VisibleSet();

		/**
		Removes all elements from this set.
		*/
		void Clear();
		void CopyElements(Vector<VisibleElement> newElements);
	};
}