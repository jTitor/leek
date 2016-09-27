#pragma once
#include "IStatDisplayer.h"

namespace LeEK
{
	class IStatCollector
	{
	protected:
		IStatDisplayer* displayer;
	public:
		static void SetStatDisplayer(IStatDisplayer* displayer);
		static void WriteStats();
		static void WriteCSV(DataStream* file);
		static void DumpCSV(const Path& path);
	}
}