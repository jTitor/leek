#pragma once
#include "Datatypes.h"

namespace LeEK
{
	template<typename T>
	class LinkedListNode
	{
	public:
		T Value;
		LinkedListNode<T>* Next;
		
		LinkedListNode(const T& pValue)
		{
			Value = pValue;
		}
	};

	/**
	Implements a singly-linked list.
	*/
	class LinkedList
	{
	private:
		//LinkedListNode* root;
	};
}
