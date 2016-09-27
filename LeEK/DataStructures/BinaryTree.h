#pragma once
#include "Datatypes.h"

namespace LeEK
{
	//Node of a binary tree.
	//Nothing special here.
	class BTreeNode
	{
	public:
		BTreeNode* Parent;
		BTreeNode* Left;
		BTreeNode* Right;

		BTreeNode()
		{
			Parent = NULL;
			Left = NULL;
			Right = NULL;
		}
		virtual ~BTreeNode() {}

		bool IsLeaf() { return (Left == NULL) && (Right == NULL); }
		bool IsRoot() { return (!Parent);}
	};

	//deletes a given node and all children of the given node.
	void DeleteBTree(BTreeNode* root);

	class SortedBTreeNode
	{
	};
}
