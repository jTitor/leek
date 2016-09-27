#pragma once
#include <Datatypes.h>

namespace LeEK
{
	typedef U32 idT;
	class IDManager
	{
	private:
		idT nextID;
	public:
		IDManager(void);
		~IDManager(void);
		idT GetNextID();
	};
}