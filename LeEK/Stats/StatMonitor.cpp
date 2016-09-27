#include "StatMonitor.h"
#include "Logging/Log.h"
#include "FileManagement/DataStream.h"
#include "FileManagement/IStrStream.h"
#include "Time/DateTime.h"
#include "AllocStats.h"
using namespace LeEK;

StatMonitor::StatMonitor(void)
{
	shouldPrint = true;
}


StatMonitor::~StatMonitor(void)
{
}

void StatMonitor::WriteStatLn(const char* profLine)
{
	Log::D(profLine);
}

void StatMonitor::Startup()
{
	//init individual stat units
	fps.Startup();
	//set default values
	secSinceStatUpdate = 0.0f;
	SetUpdateRate(1.0f);
	strBuf[0] = 0;

	//also set this as the profiler's handler
	ProfileSample::SetStatDisplayer(this);
	OverheadStats::SetStatDisplayer(this);
	AllocStats::SetStatDisplayer(this);
}

void StatMonitor::PrintStats()
{
	Log::D("Stat Summary");
	Log::D(Allocator::FindAllocSummary());
	sprintf_s(strBuf, BUF_SIZE, "FPS: %.1f", fps.FPS());
	Log::D(strBuf);
	ProfileSample::WriteStats();
	OverheadStats::WriteStats();
	AllocStats::WriteStats();
}

void StatMonitor::Update(const GameTime& time)
{
	fps.Update(time);
	secSinceStatUpdate += time.ElapsedGameTime().ToSeconds();
	//if it's time for an update, report stats!
	if(shouldPrint && secSinceStatUpdate >= secUpdateRate)
	{
		//clear the screen
		plat->ClearTerm();
		//remember to temporarily disable buffering, otherwise it REALLY fills up the logs.
		Log::SetBufferPaused(true);
		PrintStats();
		Log::SetBufferPaused(false);
		secSinceStatUpdate -= secUpdateRate;
	}
}

void StatMonitor::DumpStatsCSV(const Path& path)
{
	IStrStream* logFile = (IStrStream*)Filesystem::OpenFile(path);
	LogD("Dumping debug stats...");
	if(!logFile)
	{
		LogW("Couldn't open debug stat file!");
		return;
	}
	//try to append to any existing log
	logFile->SeekToEnd();
	if(logFile->FileSize() > 0)
	{
		logFile->WriteLine();
	}
	logFile->WriteLine(String("Stat Summary\nDumped at time: ") + DateTime::GetCurrLocalTime());
	logFile->WriteLine("Average FPS");
	logFile->WriteLine(StrFromVal(fps.FPS()));
	logFile->WriteLine("Profiler");
	ProfileSample::WriteCSV(logFile);
	logFile->WriteLine("Dynamic Allocs");
	Allocator::WriteAllocsCSV(logFile);
	logFile->WriteLine("Overhead and Misc");
	OverheadStats::WriteCSV(logFile);
	AllocStats::WriteCSV(logFile);
	//we're done, remember to close the file
	logFile->Close();
	LogD("Debug stat dump complete.");
}

void StatMonitor::SetUpdateRate(F32 val)
{
	F32 newVal = Math::Max(1.0f / 1000, val);
	secUpdateRate = val;
	fps.SetUpdateRate(val);
}