#pragma once
#ifdef WIN32
#include "AudioManager.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "SoundInstance.h"
#include "Platforms/Win32Platform.h"

namespace LeEK
{
	class DSoundAudioWrapper : public AudioWrapper
	{
	protected:
		IDirectSound8* dSound;
		const Win32Platform* plat;

		//builds DirectSound buffers according to given parameters.
		//priFrequency should be given as the freqency in hertz - 44.1 kHZ = 44100, for example.
		HRESULT SetPrimaryBufferFormat(DWORD numPriChannels, DWORD priFrequency, DWORD priBitRate);
	public:
		DSoundAudioWrapper(const Win32Platform* pPlat);

		bool Active() { return dSound != NULL; }

		bool Initialize();
		void Shutdown();

		ISoundInstance* InitInstance(ResPtr resource);
		void ReleaseInstance(ISoundInstance* instance);
	};

	class DSoundInstance : public SoundInstance
	{
	private:
		LPDIRECTSOUNDBUFFER buf;

		HRESULT fillBuffer();
		HRESULT restoreBuffer(BOOL* wasRestored);
	public:
		DSoundInstance(LPDIRECTSOUNDBUFFER bufParam, ResPtr resPtr);

		void* GetImplementationData();
		bool Restore();

		bool Play(I32 volumeParam, bool loopParam);
		bool Pause();
		bool Stop();
		bool Resume();

		bool TogglePause();
		bool IsPlaying();
		void SetVolume(I32 volumeParam);
		F32 GetProgress();
	};
}
#endif