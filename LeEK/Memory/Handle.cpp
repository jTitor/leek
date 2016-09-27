#include "Handle.h"
#include "DebugUtils/Assertions.h"
#include "DataStructures/STLContainers.h"

using namespace LeEK;

Vector<void*> handleMap = Vector<void*>();
//typedef Vector<void*>::iterator MapIt;
//typedef Vector<void*>::const_iterator ConstMapIt;
Handle nextHnd = 1;

Handle nextHandle()
{
	L_ASSERT(nextHnd != MAX_HND);
	Handle res = nextHnd;
	nextHnd++;
	return res;
}

void* doGetPtr(Handle hnd)
{
	//remember that handles start at 1,
	//but their position in the vector starts at 0.
	if(hnd == 0)
	{
		return NULL;
	}
	return handleMap[hnd - 1];
}

void setHnd(Handle hnd, void* ptr)
{
	if(hnd == 0)
	{
		return;
	}
	handleMap[hnd - 1] = ptr;
}

void* HandleMgr::GetPointer(const Handle& hnd)
{
	if(hnd != 0 && hnd <= handleMap.size())
	{
		//Should really fix this -
		//would be faster to make the handles
		//indices into an array.
		//Then if a handle's deleted, maybe add it to some
		//ID pool.
		return doGetPtr(hnd);
	}
	return 0;
}

Handle HandleMgr::RegisterPtr(void* ptr)
{
	if(ptr)
	{
		Handle hnd = FindHandle(ptr);//nextHandle();
		if(hnd)
		{
			return hnd;
		}
		hnd = nextHandle();
		//Put the pointer at the end of the list.
		//Since the vector's 0-indexed, the size
		//will serve as the handle to that pointer.
		handleMap.push_back(ptr);
		return handleMap.size();
	}
	return 0;
}

Handle HandleMgr::FindHandle(void* ptr)
{
	//must do a search of all handles
	//returns the FIRST handle pointing to given pointer
	/*
	for(ConstMapIt it = handleMap.begin(); it != handleMap.end(); ++it)
	{
		if(*it == ptr)
		{
			return it.;
		}
	}
	*/
	for(int i = 0; i < handleMap.size(); ++i)
	{
		void* hndPtr = handleMap[i];
		if(hndPtr == ptr)
		{
			//remember, handles are 1-indexed.
			return i+1;
		}
	}
	return 0;
}

void HandleMgr::RemoveHandle(const Handle& hnd)
{
	setHnd(hnd, NULL);
	//handleMap.erase(hnd);
}

void HandleMgr::RemovePtr(void* ptr)
{
	for(int i = 0; i < handleMap.size(); ++i)
	{
		//If the handle's pointer matches, remove it.
		//No point worrying about if it's been deleted,
		//since the memory's not ours to manipulate now.
		if(handleMap[i] == ptr)
		{
			handleMap[i] = NULL;
			//Notify that this slot opened up, maybe.
		}
	}
}

void HandleMgr::DeleteHandle(const Handle& hnd)
{
	CustomDelete(GetPointer(hnd));
	RemoveHandle(hnd);
}

void HandleMgr::MoveHandle(const Handle& hnd, void* newPtr)
{
	//remove the handle if the new pointer is null?
	if(hnd != 0 && hnd <= handleMap.size())
	{
		/*
		MapIt hndIt = handleMap.find(hnd);
		if(hndIt != handleMap.end())
		{
			hndIt->second = newPtr;
		}*/
		setHnd(hnd, newPtr);
	}
}

void HandleMgr::NotifyRegionPurged(void* regionStart, size_t regionSize)
{
	//Iterate through each handle
	for(int i = 0; i < handleMap.size(); ++i)
	{
		//If the handle's pointer in the region, remove it.
		//No point worrying about if it's been deleted,
		//since the memory's not ours to manipulate now.
		size_t regionStartCast = (size_t)regionStart;
		size_t ptrCast = (size_t)handleMap[i];
		if(	ptrCast >= regionStartCast &&
			ptrCast <= regionStartCast + regionSize)
		{
			handleMap[i] = NULL;
			//Notify that this slot opened up, maybe.
		}
	}
}
