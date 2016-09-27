#include "IDManager.h"
#include "DebugUtils/Assertions.h"

using namespace LeEK;

const idT INVALID_ID = ~0;

IDManager::IDManager(void) : nextID(0)
{
}


IDManager::~IDManager(void)
{
}

idT IDManager::GetNextID()
{
	L_ASSERT(nextID < INVALID_ID);
	idT res = nextID;
	nextID++;
	return res;
}