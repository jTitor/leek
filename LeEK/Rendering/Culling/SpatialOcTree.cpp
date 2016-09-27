#include "SpatialOcTree.h"

using namespace LeEK;

Vector3 SpatialOcTree::getValuePosition(const VisibleElement data)
{
	if(!data.Spatial)
	{
		return Vector3::Zero;
	}
	//We need the world position, of course.
	return data.Spatial->GetWorldTransform().Position();
}

bool SpatialOcTree::compareValue(const Value& val, const Node& node)
{
	//Since everything's handles, all we can go on is handle comparisons.
	//Check that the passed handle matches the node's handle.
	if(val.Spatial == node.Data().Spatial)
	{
		return true;
	}
	return false;
}

bool SpatialOcTree::shouldInsert(const Value& pData)
{
	//if this isn't a visible element, quit right now.
	if(!pData.Spatial || pData.Spatial->GetContainMode() != SpatialNode::NODE_LEAF)
	{
		return false;
	}
	return true;
}