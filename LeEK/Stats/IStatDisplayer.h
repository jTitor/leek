#pragma once

namespace LeEK
{
	class IStatDisplayer
	{
	public:
		virtual void WriteStatLn(const char* line) = 0;
	};
}