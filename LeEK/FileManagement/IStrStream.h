#pragma once
#include "Datatypes.h"
#include "FileManagement/DataStream.h"
#include "Strings/String.h"

namespace LeEK
{
	//since the allocator will completely lose it if we use strings in it
	//(since strings have to be defined relative to it at the moment),
	//we use this wrapper to get string functionality
	class IStrStream : public DataStream
	{
	public:
		virtual ~IStrStream(void) {}
		virtual FileSz WriteLine(const String& = "") = 0;
	};
}