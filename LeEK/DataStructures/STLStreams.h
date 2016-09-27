#pragma once
#include <istream>

namespace LeEK
{
	struct MemBuf : std::streambuf
	{
		MemBuf(char* bufStart, size_t bufSize)
		{
			this->setg(bufStart, bufStart, bufStart + bufSize);
		}
	};
}