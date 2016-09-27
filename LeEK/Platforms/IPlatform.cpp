#include "Platforms/IPlatform.h"

using namespace LeEK;

bool IPlatform::ShouldLockMouse()
{
	return shouldLockMouse;
}

void IPlatform::SetShouldLockMouse(bool val)
{
	shouldLockMouse = val;
}