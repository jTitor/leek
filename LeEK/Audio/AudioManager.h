#pragma once
#include "ISoundInstance.h"

namespace LeEK
{
	class ISoundInstance;
	enum AudioType { OPEN_AL, DIRECTSOUND };

	class IAudioWrapper
	{
	public:
		~IAudioWrapper() {}

		virtual bool Active() = 0;
		virtual ISoundInstance* InitInstance(ResPtr soundRes) = 0;
		virtual void ReleaseInstance(ISoundInstance* instance) = 0;

		virtual void StopAllSounds() = 0;
		virtual void PauseAllSounds() = 0;
		virtual void ResumeAllSounds() = 0;

		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;
	};

	//implements platform-generic aspects of IAudioWrapper.
	class AudioWrapper : public IAudioWrapper
	{
	protected:
		typedef List<ISoundInstance*> SoundInstanceList;
		SoundInstanceList samples;
		bool allPaused;
		bool initialized;

	public:
		AudioWrapper();

		virtual void StopAllSounds();
		virtual void PauseAllSounds();
		virtual void ResumeAllSounds();

		virtual void Shutdown();

		static bool HasSoundCard();
		bool IsPaused() { return allPaused; }
	};
}