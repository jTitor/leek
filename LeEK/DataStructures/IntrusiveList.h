#pragma once
#include "Datatypes.h"

namespace LeEK
{
	//idea is, the object stores the list's node inside itself.
	//template<typename T>
	struct IntrusiveListNode
	{
		IntrusiveListNode* Next;
		IntrusiveListNode* Prev;
	};

	//template<typename T>
	class IntrusiveList
	{
	private:
		IntrusiveListNode root;
	public:
		typedef IntrusiveListNode* nodePtr;
		inline nodePtr Front() { return root.Next; }
		inline nodePtr Back() { return root.Prev; }
		inline nodePtr Root() { return &root; }
		inline bool Empty() { return Front() == &root; }

		IntrusiveList()
		{
			root.Next = &root;
			root.Prev = &root;
		}

		void Init()
		{
			root.Next = &root;
			root.Prev = &root;
		}

		//inserts a node between two other given nodes.
		//Prev will be the node's Previous node, Next will be the node's Next node.
		//Kinda obvious in retrospect.
		void Add(nodePtr node,  nodePtr prev, nodePtr next);

		void Remove(nodePtr node);

		//add to the head of the list
		void AddToFront(nodePtr node) { Add(node, &root, root.Next); }
		void AddToBack(nodePtr node) { Add(node, root.Prev, &root); }
		nodePtr PopFront()
		{
			nodePtr result = Front();
			Remove(result);
			return result;
		}
		nodePtr PopBack()
		{
			nodePtr result = Back();
			Remove(result);
			return result;
		}
	};
}
