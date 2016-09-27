#include "StdAfx.h"
#include "GameTime.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
#include "DebugUtils/Assertions.h"
using namespace LeEK;
U32 GameTime::MAX_FPS = 50;
U32 GameTime::MAX_FRAME_SKIPS = 0;

#ifdef WIN32
LARGE_INTEGER liStore; // storage variable used in Win32 implementation
#endif

GameTime::GameTime(void) : prevTime(0), currTime(0), totalGameTime(0), sleepTime(0), framesSkipped(0)
{
#ifdef WIN32
	//if we're under Win32, initialize the CPU frequency
	//if this somehow doesn't work, freak out
	bool result = QueryPerformanceFrequency(&liStore);
	L_ASSERT(result && "Couldn't get CPU frequency!");
	cpuFreq = double(liStore.QuadPart) / 1000.0;
	//now prime the clock
	QueryPerformanceCounter(&liStore);
	prevTime = liStore.QuadPart;
	currTime = liStore.QuadPart;
	lastAvg = 0;
#else
	struct timeval tv;
	gettimeofday( &tv, 0 );
	cpuFreq = 1000;
	prevTime = tv.tv_usec + tv.tv_sec * 1000;
	currTime = prevTime;
#endif
	
	Tick();
}

GameTime::~GameTime(void)
{
}

void GameTime::Tick()
{
	prevTime = currTime;
#ifdef WIN32
	QueryPerformanceCounter(&liStore);
	currTime = liStore.QuadPart;
#else
	struct timeval tv;
	gettimeofday( &tv, 0 );
	currTime = tv.tv_usec + tv.tv_sec * 1000;
#endif
	lastAvg = (lastAvg + (currTime - prevTime)) / 2;
	totalGameTime += (currTime - prevTime);
	sleepTime = CycleLengthDelta().ToMilliseconds();
	//reset the frame skip count
	framesSkipped = 0;
}
