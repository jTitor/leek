#include "OcTree.h"

using namespace LeEK;

Vector3 BoundsOcTree::getValuePosition(Value data)	
{
	return data->Center();
}

bool BoundsOcTree::compareValue(const Value& val, const Node& node)
{
	//Don't have an equality test between matching bounds types;
	//If pointers don't match, check that the bounds center,
	//point count, radius and bounds type match.
	if(node.Data() == val)
	{
		return true;
	}
	Value nodeVal = node.Data();
	if(	nodeVal->Center() == val->Center() &&
		nodeVal->GetNumPoints() == val->GetNumPoints() &&
		nodeVal->Radius() == val->Radius() &&
		nodeVal->GetType() == val->GetType())
	{
		return true;
	}
	return false;
}

Vector3 DebugBoundsOcTree::getValuePosition(Value data)	
{
	return data->Center();
}