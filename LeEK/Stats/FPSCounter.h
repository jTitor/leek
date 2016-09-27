#pragma once
#include "Datatypes.h"
#include "Time/GameTime.h"

namespace LeEK
{
	class FPSCounter
	{
	private:
		F32 fps;
		F32 timeSinceUpdate;
		F32 updateRate;
		U32 frameCount;
	public:
		FPSCounter(void);
		~FPSCounter(void);
		inline const F32 FPS() const { return fps; };
		inline const F32 UpdateRate() const { return updateRate; }
		void SetUpdateRate(F32 val) { updateRate = val; }
		void Startup();
		void Update(const GameTime& time);
	};
}
