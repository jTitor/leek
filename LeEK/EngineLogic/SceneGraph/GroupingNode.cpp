#include "GroupingNode.h"
#include "Rendering/Culling/Culler.h"


using namespace LeEK;

GroupingNode::GroupingNode(U32 reserve, U32 growRate)
{
	containerMode = NODE_CONTAINER;
	children = Vector<SpatialHnd>();
	children.reserve(reserve);
}

GroupingNode::~GroupingNode()
{
}

U32 GroupingNode::GetReserve() const
{
	//current implementation doesn't use reserve space
	return children.capacity();//0;
}

U32 GroupingNode::GetNumChildren() const
{
	//cf. GetReserve()
	return children.size();
}

I32 GroupingNode::AttachChild(SpatialHnd child)
{
	//Simple, put child in list
	//and return its index in the list.
	//Elements can be null, so linearly search for a free space.
	for(int i = 0; i < children.size(); ++i)
	{
		if(children[i] == 0)
		{
			children[i] = child;
			return i;
		}
	}
	//If no element's free, push to the back.
	children.push_back(child);
	return children.size() - 1;
}

I32 GroupingNode::DetachChild(SpatialHnd child)
{
	childrenIt c = children.begin();
	//Start at 1 so the returned index doesn't have to be adjusted!
	I32 returnedIndex = 0;
	while(c != children.end())
	{
		//if addresses match, NULL out the element
		if(*c == child)
		{
			children[returnedIndex] = 0;//.erase(c++);
			//and return the index
			return returnedIndex;
		}
		else
		{
			//otherwise, just go to next child
			//and increment the index counter
			++c;
			++returnedIndex;
		}
	}
	//couldn't find child, return invalid index
	return -1;
}

SpatialHnd GroupingNode::DetachChildAt(I32 index)
{
	if(index < 0 || index >= children.size())
	{
		return 0;
	}
	childrenIt c = children.begin();
	for(U32 i = 0; i < index; ++i)
	{
		++c;
	}
	SpatialHnd result = *c;
	children.erase(c);
	return result;
}

SpatialHnd GroupingNode::SetChild(I32 index, SpatialHnd child)
{
	if(index < 0 || index >= children.size())
	{
		return 0;
	}
	SpatialHnd prevChild = children[index];
	children[index] = child;
	return prevChild;
}

SpatialHnd GroupingNode::GetChild(I32 index)
{
	if(index < 0 || index >= children.size())
	{
		return 0;
	}
	return children[index];
}

//Interface implementation.
void GroupingNode::OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans)
{
	//If this node has shaders attached, they apply to all children.
	//Note this in the visible set by attaching a sentinel value.
	if(GetNumLocalShaders() > 0)
	{
		culler.Insert(thisHnd, localShaders.at(0));
	}

	//Insert children into visible set.
	for(int i = 0; i < children.size(); ++i)
	{
		children[i]->OnGetVisibleSet(culler, shouldNotCull, pRecalcTrans);
	}

	//Then note the end of the affected nodes.
	if(GetNumLocalShaders() > 0)
	{
		culler.Insert(0, 0);
	}
}
