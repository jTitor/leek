#pragma once
#include "Datatypes.h"

class Event
{
private:
	//holder for extra data
	void* extraData;
	U32 eventType;
public:
	Event(void);
	~Event(void);
};

