#include "FPSCounter.h"

using namespace LeEK;

FPSCounter::FPSCounter(void) : fps(0), timeSinceUpdate(0), updateRate(1), frameCount(0)
{
}


FPSCounter::~FPSCounter(void)
{
}

void FPSCounter::Startup()
{
}

void FPSCounter::Update(const GameTime& time)
{
	frameCount++;
	timeSinceUpdate += time.ElapsedGameTime().ToSeconds();
	if(timeSinceUpdate > updateRate)
	{
		fps = frameCount / timeSinceUpdate;
		frameCount = 0;
		timeSinceUpdate -= updateRate;
	}
	
	//fps = 1.0f / time.ElapsedGameTime().ToSeconds();
}

