#include "IntrusiveList.h"

using namespace LeEK;

void IntrusiveList::Add(IntrusiveListNode* node,  IntrusiveListNode* prev, IntrusiveListNode* next)
{
	node->Next = next;
	node->Prev = prev;
	if(next)
	{
		next->Prev = node;
	}
	if(prev)
	{
		prev->Next = node;
	}
}

void IntrusiveList::Remove(IntrusiveListNode* node)
{
	if(node->Next)
	{
		node->Next->Prev = node->Prev;
	}
	if(node->Prev)
	{
		node->Prev->Next = node->Next;
	}
	node->Next = NULL;
	node->Prev = NULL;
}
