#include "OcTreeNode.h"
#include "Rendering/Bounds/AABBBounds.h"

using namespace LeEK;

OcTreeNodeBase::OcTreeNodeBase(OcTreeNodeBase* pParent)
{
	Parent = pParent;
	//Increment level if parent exists
	if(Parent)
	{
		Level = Parent->Level + 1;
	}
	else
	{
		Level = 0;
	}
	//regionCenter = pCenter;
	NumChildren = 0;
	//initialize children
	for(int i = 0; i < LOCATION_COUNT; ++i)
	{
		Children[i] = NULL;
	}
}

OcTreeNodeBase::~OcTreeNodeBase(void)
{
	//recurse through children and delete
	for(int i = 0; i < LOCATION_COUNT; ++i)
	{
		OcTreeNodeBase* child = Children[i];
		//LArrayDelete(child);
		LDelete(child);
	}
}

bool OcTreeNodeBase::IsLeaf() const
{
	return NumChildren == 0;
}

bool OcTreeNodeBase::IsRoot() const
{
	return Parent == NULL;
}