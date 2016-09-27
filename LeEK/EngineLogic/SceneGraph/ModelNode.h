#pragma once
#include "SpatialNode.h"
#include "Rendering/Model.h"

namespace LeEK
{
	/**
	Denotes geometry used in a scene.
	Although each ModelNode can only be a child of one other node,
	ModelNodes can be initialized with the same underlying Geometry object
	to share mesh data.
	*/
	class ModelNode : public SpatialNode
	{
		TypedHandle<Model> model;
	public:
		ModelNode() : model() {}
		void SetGeometry(TypedHandle<Model> modelHnd);
		//void SetGeometry(const Geometry* geomPtr);
		TypedHandle<Model> GetModel();

		//TODO
		void OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans);
	};
}