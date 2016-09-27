#include "BinaryTree.h"
#include "Memory/Allocator.h"

using namespace LeEK;

void LeEK::DeleteBTree(BTreeNode* root)
{
	//Check that this is a leaf node.
	//If not, recurse through the children
	if(root->Left)
	{
		DeleteBTree(root->Left);
		//note that the child node is gone
		root->Left = NULL;
	}
	if(root->Right)
	{
		DeleteBTree(root->Right);
		root->Right = NULL;
	}
	//should now be a leaf node
	L_ASSERT(root->IsLeaf() && "Node still has children after deleting all children!")
	//delete it and quit
	CustomDelete(root);
	return;
}