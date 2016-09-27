#include "OcTreeCuller.h"

using namespace LeEK;

const int DEF_REGION_SIZE = 1000;

OcTreeCuller::OcTreeCuller(void) : ocTree(DEF_REGION_SIZE)
{
	nodeToElem = NodeToElemMap();
}

OcTreeCuller::~OcTreeCuller(void)
{
}

void OcTreeCuller::CalcVisibleSet()
{
	//traversing the octree will create the visible set we need.
	Vector<VisibleElement> newVisSet = ocTree.FindAllInBounds(camera->GetWorldFrustum());

	//Set the found elements as the visible set's elements.
	visible.Elements = newVisSet;
}

SpatialOcTree::Node* OcTreeCuller::findVisElem(TypedHandle<SpatialNode> node)
{
	if(nodeToElem.find(node) == nodeToElem.end())
	{
		return NULL;
	}
	SpatialOcTree::Node* visElem = nodeToElem[node];
	return visElem;
}

void OcTreeCuller::Insert(TypedHandle<SpatialNode> node, 
			TypedHandle<Shader> globalShader)
{
	//do nothing; we will not loop over elements like this.
}

void OcTreeCuller::OnSceneNodeAdded(TypedHandle<SpatialNode> newNode)
{
	//If this is a geometry node, insert it into the octree.
	if(newNode->GetContainMode() != SpatialNode::NODE_LEAF)
	{
		return;
	}
	newNode->OnInsert();
	VisibleElement newElem = VisibleElement(newNode, newNode->FindGlobalShaders());
	SpatialOcTree::Node* treeNode = ocTree.Insert(newElem);
	if(!treeNode)
	{
		return;
	}
	//maybe place the resulting node in a map?
	nodeToElem[newNode] = treeNode;
}

void OcTreeCuller::OnSceneNodeUpdated(TypedHandle<SpatialNode> node)
{
	if(node->GetContainMode() != SpatialNode::NODE_LEAF)
	{
		return;
	}
	//Find the desired node.
	auto visElem = findVisElem(node);
	if(visElem == NULL)
	{
		return;
	}
	//Notify the octree that the node should be updated.
	ocTree.UpdateNode(visElem);
}

void OcTreeCuller::OnSceneNodeMoved(TypedHandle<SpatialNode> movedNode)
{
	//The hierarchy has changed,
	//but the node has not moved in space.
	//Anything before this should have updated the node's
	//local transforms to maintain its world transform;
	//however, we now need to update the shaders of the visual element.
	if(movedNode->GetContainMode() != SpatialNode::NODE_LEAF)
	{
		return;
	}
	auto visElem = findVisElem(movedNode);
	if(visElem == NULL)
	{
		return;
	}
	//Reload the visible element's global shaders.
	visElem->WritableData().GlobalShaders = movedNode->FindGlobalShaders();
}

void OcTreeCuller::OnSceneNodeRemoved(TypedHandle<SpatialNode> node)
{
	if(node->GetContainMode() != SpatialNode::NODE_LEAF)
	{
		return;
	}
	//Remove the node from the octree...
	auto visElem = findVisElem(node);
	//If the node's not in the lookup table,
	//it shouldn't be in the octree either, so quit
	if(visElem == NULL)
	{
		return;
	}
	VisibleElement toRemove = VisibleElement(node);
	ocTree.RemoveAt(toRemove, visElem);
	//Also remove the visible element from the lookup map.
	//Because of findVisElem(), we know the map contains node.
	nodeToElem.erase(node);
}