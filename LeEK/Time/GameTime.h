#pragma once
#include "Datatypes.h"
#include "Duration.h"

namespace LeEK
{
	class GameTime
	{
	private:
		U64 prevTime, currTime;
		U64 lastAvg;
		U64 totalGameTime;
		F64 sleepTime; // the length of time a game loop cycle has to spare
//#ifdef WIN32
//#endif
		F64 cpuFreq; //cpu frequency in tick/ms, used in Win32 implementation
		static U32 MAX_FPS; //= 50
		static U32 MAX_FRAME_SKIPS; //= 0
		U32 framesSkipped;

	public:
		//make this private, make this a singleton system?
		//there should be only one hi-res timer.
		GameTime(void);
		~GameTime(void);
		/**
		 * Updates the GameTime. Call once after every frame has passed.
		 */
		void Tick();

		/**
		 * Returns the number of nanoseconds passed since the last frame.
		 * @return
		 * The number of nano seconds passed since the last frame.
		 */
		Duration ElapsedGameTime() 
		const {
#ifdef WIN32
			return Duration((currTime - prevTime) / cpuFreq); 
#else
			return Duration(currTime - prevTime);
#endif
		}
	
		Duration AverageElapsedGameTime()
		const {
			U64 avgDur = (currTime + lastAvg) / 2;
			#ifdef WIN32
			return Duration(lastAvg / cpuFreq); 
#else
			return Duration(lastAvg);
#endif
		}

		/**
		 * Returns the number of nanoseconds since the game was started.
		 * @return
		 * The number of nanoseconds since the game was started.
		 */
		Duration TotalGameTime() 
		const { 
#ifdef WIN32
			return Duration(totalGameTime / cpuFreq); 
#else
			return Duration(totalGameTime);
#endif
		}

		/*
		 * Fixed time step functions
		 */
		inline void SetDesiredFPS(int fps) { MAX_FPS = fps; }
	
		/**
		 * Returns the number of nanoseconds the current frame is early by.
		 * @return
		 */
		inline Duration CycleLengthDelta() 
		const {
#ifdef WIN32
			return Duration((FramePeriod().ToMilliseconds() - (currTime - prevTime)) / cpuFreq); 
#else
			return Duration(FramePeriod().ToNanoseconds() - (currTime - prevTime));
#endif
		}
	
		/**
		 * @return
		 * The length of time a game loop cycle has to spare.
		 */
		inline Duration SleepTime() 
		const { 
#ifdef WIN32
			return Duration(sleepTime);// / cpuFreq); 
#else
			return Duration(sleepTime);
#endif
		}
	
		inline bool CanSkipFrame() const { return framesSkipped < MAX_FRAME_SKIPS; }
	
		/**
		 * @param skipLimit
		 * The maximum number of frames the Game can skip.
		 */
		inline void SetFrameSkipLimit(int skipLimit)
		{
			if(skipLimit >= 0)
			{
				MAX_FRAME_SKIPS = skipLimit;
			}
		}
	
		void SkipFrame()
		{
			if((*this).CanSkipFrame())
			{
				sleepTime += FramePeriod().ToNanoseconds();
				framesSkipped++;
			}
			else
			{
				return;
			}
		}
	
		/**
		 * @return
		 * The Game's optimal frame cycle period in ms.
		 */
		inline Duration FramePeriod()
		const { 
			return Duration(1000000L / MAX_FPS); 
		}

		static GameTime* GetInstance();
	};
}

