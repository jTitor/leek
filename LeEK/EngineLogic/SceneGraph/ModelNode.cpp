#include "ModelNode.h"
#include "Rendering/Culling/Culler.h"

using namespace LeEK;

void ModelNode::SetGeometry(TypedHandle<Model> modelHnd)
{
	model = modelHnd;
}

TypedHandle<Model> ModelNode::GetModel()
{
	return model;
}

void ModelNode::OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool recalcTrans)
{
	//Just place this into the culler; it'll know what to do
	culler.Insert(thisHnd, 0);
}