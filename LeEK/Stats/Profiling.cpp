#include "Profiling.h"
#include "DebugUtils/Assertions.h"
#include "Logging/Log.h"
#include "Math/MathFunctions.h"
#include "FileManagement/DataStream.h"
#ifdef WIN32
#include <Windows.h>
#endif
using namespace LeEK;

#ifdef WIN32
static LARGE_INTEGER liStore; // storage variable used in Win32 implementation
static F64 cpuFreq = 0;

//gets CPU frequency, as ticks/ms.
void initCPUFreq()
{
	bool result = QueryPerformanceFrequency(&liStore);
	L_ASSERT(result && "Couldn't get CPU frequency!");
	cpuFreq = double(liStore.QuadPart) / 1000.0;
}
#endif

I32 ProfileSample::LastOpenedSample = -1;
I32 ProfileSample::NumOpenSamples = 0;
U64 ProfileSample::RootCallStart = 0;
U64 ProfileSample::RootCallEnd = 0;
IStatDisplayer* ProfileSample::statDisp = NULL;
static const U32 MAX_BUF_CHARS = 1024;
static char lineBuf[MAX_BUF_CHARS] = {0};
bool ProfileSample::samplesReady = false;
struct ProfileSample::profSampleData ProfileSample::samples[MAX_PROF_SAMPLES];

//returns current time in milliseconds.
inline U64 getTime()
{
	#ifdef WIN32
	QueryPerformanceCounter(&liStore);
	return (U64)liStore.QuadPart;
	#endif
}

inline F32 tickToMs(U64 tick)
{
	#ifdef WIN32
	if(cpuFreq == 0)
	{
		initCPUFreq();
	}
	return (F32)(((F64)tick)/cpuFreq);
	#endif
}

void ProfileSample::writeLn(F32 minPct, F32 avgPct, F32 maxPct, U32 numCalls, U32 numRecs, const char* tag, I32 numParents)
{
	char indentTag[128];
	I32 indentLevel;
	for(indentLevel = 0; indentLevel < numParents; indentLevel++)
	{
		indentTag[indentLevel] = ' ';
	}
	//append the null to the indentation
	indentTag[indentLevel] = 0;
	sprintf_s(	lineBuf, MAX_BUF_CHARS, "%3.1f\t| %3.1f\t| %3.1f\t| %3d\t| %3d\t| %s%s", 
				minPct, avgPct, maxPct, numCalls, numRecs, indentTag, tag);
	statDisp->WriteStatLn(lineBuf);
}

ProfileSample::ProfileSample(const char* tag)
{
	if(!samplesReady)
	{
		for(I32 i = 0; i < MAX_PROF_SAMPLES; ++i)
		{
			samples[i] = profSampleData();
		}
		samplesReady = true;
	}

	sampleIndex = 0;
	parentIndex = 0;

	//search through the sample array for a sample matching the tag
	I32 nextEmptyIndex = -1;
	for(I32 i = 0; i < MAX_PROF_SAMPLES; ++i)
	{
		//also store the first empty index found
		if(nextEmptyIndex < 0 && !samples[i].IsValid)
		{
			nextEmptyIndex = i;
		}
		else
		{
			if(samples[i].Tag && strcmp(samples[i].Tag, tag) == 0)
			{
				//if found, update the data
				L_ASSERT(!samples[i].IsOpen && "Tried to profile a sample already being profiled");
				//store the sample's index
				sampleIndex = i;
				parentIndex = LastOpenedSample;
				LastOpenedSample = i;
				//and we know how many blocks are below this sample's from the open count
				samples[i].NumParents = NumOpenSamples;
				//inc, since this sample is now open
				++NumOpenSamples;
				samples[i].IsOpen = true;
				++samples[i].NumCalls;
				//now store the current time to start the sample
				samples[i].StartTime = getTime();
				//if this sample has no parent, must be the main loop sample;
				//store it in the global timestamp
				if(parentIndex < 0)
				{
					RootCallStart = samples[i].StartTime;
				}
				return;
			}
		}
	}
	//if there is none, make a new sample at the stored index
	//and init the data
	L_ASSERT(nextEmptyIndex >= 0 && "Profiler has no more empty sample slots!");
	L_ASSERT(sampleIndex >= 0);
	L_ASSERT(!samples[nextEmptyIndex].IsValid);
	L_ASSERT(strcmp(samples[nextEmptyIndex].Tag, tag) != 0);
	samples[nextEmptyIndex].IsValid = true;
	samples[nextEmptyIndex].Tag = tag;
	sampleIndex = nextEmptyIndex;
	parentIndex = LastOpenedSample;
	L_ASSERT(sampleIndex >= 0);
	LastOpenedSample = nextEmptyIndex;
	samples[nextEmptyIndex].NumParents = NumOpenSamples;
	NumOpenSamples++;
	samples[nextEmptyIndex].IsOpen = true;
	samples[nextEmptyIndex].NumCalls = 1;

	//also init statistics
	samples[nextEmptyIndex].TotalTime = 0.0f;
	samples[nextEmptyIndex].ChildrenTime = 0.0f;
	samples[nextEmptyIndex].StartTime = getTime();
	L_ASSERT(sampleIndex >= 0);
	//again, init root data if needed
	if(parentIndex < 0)
	{
		RootCallStart = samples[nextEmptyIndex].StartTime;
	}
}

ProfileSample::~ProfileSample()
{
	//now we know the thing's done timing, record end time
	U64 endTime = getTime();
	samples[sampleIndex].IsOpen = false;
	//also calc duration
	F32 sampleDur = tickToMs(endTime - samples[sampleIndex].StartTime);
	//remember this elapsed time counts for any parent's time elapsed
	if(parentIndex >= 0)
	{
		samples[parentIndex].ChildrenTime += sampleDur;
	}
	//if there's no parent, must be the end of the game loop, so just save the end time
	else
	{
		RootCallEnd = endTime;
	}
	//now incr this sample's total time, go to the parent's index
	samples[sampleIndex].TotalTime += sampleDur;
	LastOpenedSample = parentIndex;
	//and reduce the number of open samples
	--NumOpenSamples;
}

void ProfileSample::SetStatDisplayer(IStatDisplayer* displayer)
{
	if(displayer)
	{
		statDisp = displayer;
	}
}

void ProfileSample::WriteStats()
{
	//TODO: make this a handler call?
	L_ASSERT(statDisp && "No output callback set");
	F32 loopDurMs = samples[0].TotalTime;//tickToMs(RootCallEnd - RootCallStart);
	statDisp->WriteStatLn("Min%\t| Avg%\t| Max%\t| Calls\t| Recs\t| ProfileTag");
	statDisp->WriteStatLn("------------------------------------------");
	for(I32 i = 0; i < MAX_PROF_SAMPLES; ++i)
	{
		//TODO: handler preprocessing
		if(samples[i].IsValid)
		{
			//calculate actual percentages here
			F32 sampleTime;
			F32 percentage;
			//need to get how long this sample's block's execution was
			//not including any child blocks
			sampleTime = samples[i].TotalTime - samples[i].ChildrenTime;
			//and recalc as a percentage of total loop execution time
			percentage = (sampleTime / loopDurMs) * 100.0f;
			//also insert into percentage data for sample
			//no need to check AvgPct's validity,
			//since it'll only be invalid when there's no samples
			F32 totPct = samples[i].AvgPct * samples[i].NumRecordings;
			totPct += percentage;
			samples[i].NumRecordings++;
			samples[i].AvgPct = totPct / samples[i].NumRecordings;
			//adjust the min and max percentages as needed
			samples[i].MinPct = Math::Min(samples[i].MinPct, percentage);
			samples[i].MaxPct = Math::Max(samples[i].MaxPct, percentage);

			//output the value
			//writeLn(samples[i].NumCalls, sampleTime, samples[i].TotalTime,
			//		samples[i].NumRecordings, samples[i].Tag, samples[i].NumParents);
			writeLn(samples[i].MinPct, samples[i].AvgPct, samples[i].MaxPct, samples[i].NumCalls,
					samples[i].NumRecordings, samples[i].Tag, samples[i].NumParents);

			//and reset sample
			samples[i].NumCalls = 0;
			samples[i].TotalTime = 0;
			samples[i].ChildrenTime = 0;
		}
		//TODO: handler postprocessing
	}
}

void ProfileSample::WriteCSV(DataStream* file)
{
	//file->WriteLine(String("Dumped at time ") + DateTime::GetCurrLocalTime());
	//file->WriteLine("Profiler Data");

	//write the tag totals first
	const U32 BUF_SIZE = 256;
	char lineBuffer[BUF_SIZE];
	const U32 MAX_INDENT = 128;
	char indentTag[MAX_INDENT];
	I32 indentLevel;
	file->WriteLine("Loop Duration (ms), Num. Samples This Frame");
	F32 mainLoopTime = samples[0].TotalTime;
	sprintf_s(lineBuffer, BUF_SIZE, "%.2f,%d", mainLoopTime, samples[0].NumRecordings);
	file->WriteLine(lineBuffer);
	file->WriteLine("Avg % of Main Loop,Execution Time (ms),Num. Calls,ProfileTag");
	for(I32 i = 0; i < MAX_PROF_SAMPLES; ++i)
	{
		if(samples[i].IsValid)
		{
			//reset indentation
			indentTag[0] = 0;
			for(indentLevel = 0; indentLevel < MAX_INDENT; indentLevel++)
			{
				if(indentLevel >= samples[i].NumParents)
				{
					break;
				}
				else
				{
					indentTag[indentLevel] = ' ';
				}
			}
			indentTag[indentLevel] = 0;
			sprintf_s(	lineBuffer, BUF_SIZE, "%.2f,%.2f,%d,%s%s", 
						samples[i].AvgPct, (samples[i].AvgPct/100.0f)*mainLoopTime,
						samples[i].NumRecordings, indentTag, samples[i].Tag);
			file->WriteLine(lineBuffer);
		}
	}
}

void ProfileSample::DumpCSV(const Path& path)
{
	//read through all the tags, and write the data in csv
	LogD("Dumping profiler data...");
	//open up a file, of course
	DataStream* logFile = Filesystem::OpenFile(path);
	if(!logFile)
	{
		LogW("Couldn't open profiler log file!");
		return;
	}
	//try to append to any existing log
	logFile->SeekToEnd();
	if(logFile->FileSize() > 0)
	{
		logFile->WriteLine();
	}
	WriteCSV(logFile);
	//we're done, remember to close the file
	logFile->Close();
	LogD("Dump complete.");
}

void ProfileSample::ResetTag(const char* tag)
{
}

void ProfileSample::ResetAll()
{
}