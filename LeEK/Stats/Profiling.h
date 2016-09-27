#pragma once
#include "Datatypes.h"
#include "Stats/IStatDisplayer.h"
#include "FileManagement/Filesystem.h"
#include <cfloat>

namespace LeEK
{
#if defined(_DEBUG) || defined(RELDEBUG)
#define ENABLE_PROFILING 1
#endif

	//controls a set of samples centered around a block of code.
	//to use, enclose as follows:
	//{
	//PROFILE((name here));
	//(code to be profiled)
	//}
	//excerpted from http://archive.gamedev.net/archive/reference/programming/features/enginuity3/index.html
	class ProfileSample
	{
	protected:
		//this sample's index in the sample array
		I32 sampleIndex;
		//the parent's sample's index
		I32 parentIndex;
		static const U32 MAX_PROF_SAMPLES = 256;
		//note that all time data is in milliseconds.
		static struct profSampleData
		{
			const char* Tag;

			U64 StartTime;		//the time this sample started
			F32 TotalTime;		//total time used by all calls of this function this frame
			F32 ChildrenTime;	//total time used by child functions this frame

			//number of times samples have been taken since creation/reset
			U32 NumRecordings;
			U32 NumCalls;		//how many times this sample's block has been called this frame

			I32 NumParents;	//number of blocks below the sample in the call stack
								//use this for indentation!
			
			//percentages of total game loop time used
			F32 AvgPct;
			F32 MinPct;
			F32 MaxPct;

			bool IsValid;		//is this sample valid data?
			bool IsOpen;		//is the sample currently being profiled?

			profSampleData()
			{
				//no samples yet, so invalidate
				IsValid = false;
				IsOpen = false;
				NumRecordings = 0;
				NumCalls = 0;
				NumParents = 0;
				AvgPct = 0;
				MinPct = 0;
				MaxPct = 0;
				StartTime = 0;
				TotalTime = 0;
				ChildrenTime = 0;
				Tag = "";
			}
		} samples[MAX_PROF_SAMPLES];
		static bool samplesReady;

		static I32 LastOpenedSample;
		static I32 NumOpenSamples;
		static U64 RootCallStart, RootCallEnd;
		static IStatDisplayer* statDisp;
		static void writeLn(F32 minPct, F32 avgPct, F32 maxPct, U32 numCalls, U32 numRecs, const char* tag, I32 numParents);
	public:
		ProfileSample(const char* tag);
		~ProfileSample();
		//could make this a handle to the displayer instead
		static void SetStatDisplayer(IStatDisplayer* displayer);
		static void WriteStats();
		static void WriteCSV(DataStream* file);
		static void DumpCSV(const Path& path);
		static void ResetTag(const char* tag);
		static void ResetAll();
	};

#ifdef ENABLE_PROFILING
#define PROFILE(tag) ProfileSample ps(tag)
#else
#define PROFILE(tag)
#endif
}
