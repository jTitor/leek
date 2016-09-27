#pragma once
#include "Datatypes.h"
#include "Time/GameTime.h"
#include "Strings/String.h"
#include "Stats/FPSCounter.h"
#include "Platforms/IPlatform.h"
#include "Stats/IStatDisplayer.h"
#include "Stats/Profiling.h"
#include "FileManagement/Filesystem.h"

namespace LeEK
{
	class StatMonitor : public IStatDisplayer
	{
	private:
		static const U32 BUF_SIZE = 1024;
		char strBuf[BUF_SIZE];
		FPSCounter fps;
		IPlatform* plat;
		F32 secSinceStatUpdate;
		F32 secUpdateRate;
		bool shouldPrint;
	public:
		StatMonitor(void);
		~StatMonitor(void);
#pragma region Properties
		inline bool ShouldPrint() { return shouldPrint; }
		inline void SetShouldPrint(bool val) { shouldPrint = val; }
		//display options?
#pragma endregion
		void WriteStatLn(const char* line);
		void Startup();
		void Update(const GameTime& time);
		void DumpStatsCSV(const Path& path);
		void PrintStats();
		const FPSCounter& GetFPSCounter() const { return fps; }
		F32 UpdateRate() const { return secUpdateRate; }
		void SetUpdateRate(F32 val);
		//char* FindStatSummary();
	};
}