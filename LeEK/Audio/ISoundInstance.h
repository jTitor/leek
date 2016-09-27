#pragma once
#include "ResourceManagement/ResourceManager.h"
#include "Datatypes.h"

namespace LeEK
{
	class ISoundInstance
	{
	public:
		virtual ~ISoundInstance() {}

		virtual void* GetImplementationData() = 0;
		virtual const ResPtr GetResource() = 0;
		//returns true only if the sound buffer is completely restored and ready for playback.
		virtual bool Restore() = 0;

		//Volume should be in the range [0, 100]
		virtual bool Play(I32 volume, bool loop) = 0;
		bool Play() { return Play(GetVolume(), IsLooping()); }
		virtual bool Pause() = 0;
		virtual bool Stop() = 0;
		virtual bool Resume() = 0;

		virtual bool TogglePause() = 0;
		virtual bool IsPlaying() = 0;
		virtual bool IsLooping() const = 0;
		virtual void SetVolume(I32 volume) = 0;
		virtual I32 GetVolume() const = 0;
		virtual F32 GetProgress() = 0;
	};
}