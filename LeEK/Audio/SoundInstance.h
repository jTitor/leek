#pragma once
#include "ISoundInstance.h"

namespace LeEK
{
	class SoundInstance : public ISoundInstance
	{
	protected:
		ResPtr resource;
		bool paused;
		bool looping;
		I32 volume;

		//forbid public construction
		SoundInstance(ResPtr resParam)
		{
			resource = resParam;
			paused = false;
			looping = false;
			volume = 0;
		}
	public:
		virtual const ResPtr GetResource() { return resource; }

		virtual bool IsPaused() const { return paused; }
		virtual bool IsLooping() const { return looping; }
		virtual void SetLooping(bool val) { looping = val; }
		virtual I32 GetVolume() const { return volume; }
	};
}
