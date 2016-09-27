#pragma once
#ifdef WIN32
#include "AudioManager.h"
#include <windows.h>
//We need the Vista/7 version of XAudio2 (v2.7).
//This means specifying that we *don't* want to search
//library folders first via quotation marks,
//otherwise we'll get the 2.8 version
//meant for Windows 8.
#include "xaudio2.h"
#include "x3daudio.h"
#include "DataStructures/STLContainers.h"
#include "SoundResource.h"
#include "SoundInstance.h"

namespace LeEK
{
	namespace
	{
		const int MAX_VOICES = 64;
	}

	class XAudioInstance;
	class XAudioAudioWrapper;

	class XAudioCallbacks : public IXAudio2VoiceCallback, public IXAudio2EngineCallback
	{
	private:
		XAudioAudioWrapper* wrapper;
	public:
		XAudioCallbacks(XAudioAudioWrapper* pWrapper);

		void STDMETHODCALLTYPE OnBufferStart(void *pBufferContext);
		void STDMETHODCALLTYPE OnBufferEnd(void *pBufferContext);
		void STDMETHODCALLTYPE OnLoopEnd(void *pBufferContext);
		//We don't directly pass streams, do we?
		void STDMETHODCALLTYPE OnStreamEnd() {}
		void STDMETHODCALLTYPE OnVoiceError(void *pBufferContext, HRESULT error);
		//These are unused for now; we don't do any voice processing.
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 bytesRequired) {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {}
		
		//Engine callbacks.
		void STDMETHODCALLTYPE OnCriticalError(HRESULT error);
		void STDMETHODCALLTYPE OnProcessingPassStart() {}
		void STDMETHODCALLTYPE OnProcessingPassEnd() {}
	};

	class XAudioAudioWrapper : public AudioWrapper
	{
	private:
		XAudioCallbacks callbacks;

		//The audio format that this audio wrapper will accept.
		AudioFormat defAudioFmt;

		Deque<ISoundInstance*> soundQueue;
		Deque<U32> freeVoiceQueue;

		IXAudio2* engine;
		IXAudio2MasteringVoice* outputVoice;
		IXAudio2SourceVoice* inputVoices[MAX_VOICES];

		/**
		Gets the next free source voice, if any is available.
		Returns null otherwise.
		*/
		I32 nextFreeVoice();
		void markVoiceFree(I32 voiceIdx);
		void updateSoundQueue();
		/**
		Attempts to play a sound instance;
		note that it does no error checking.
		Ensure that the instance has valid buffers
		and a link to a source voice before calling.
		*/
		bool tryPlay(XAudioInstance* instance);

		/**
		Callback for when an audio buffer is played.
		*/
		void xAudio2OnBufferFinish();
	public:
		XAudioAudioWrapper(AudioFormat pDefAudioFmt);

		const AudioFormat& DefaultFormat() { return defAudioFmt; }
		bool Active() { return engine != NULL; }

		bool Initialize();
		void Shutdown();

		/**
		Constructs a sound instance from a given sound file, if applicable.
		*/
		ISoundInstance* InitInstance(ResPtr resource);
		/**
		Shuts down a sound instance.
		*/
		void ReleaseInstance(ISoundInstance* instance);

		bool Play(XAudioInstance* instance);
		bool Restart(XAudioInstance* instance);
		bool Pause(XAudioInstance* instance);
		bool Resume(XAudioInstance* instance);
		bool Stop(XAudioInstance* instance);
		bool IsPlaying(XAudioInstance* instance);
		void SetVolume(XAudioInstance* instance);
		F32 GetProgress(XAudioInstance* instance);
	};

	class XAudioInstance : public SoundInstance
	{
	private:
		XAUDIO2_BUFFER buffer;
		XAudioAudioWrapper* const audio;
		I32 voiceIdx;
		//forbid copying?
		//XAudioInstance(XAudioInstance& copy);
		//XAudioInstance& operator=(XAudioInstance& copy);
		friend class XAudioAudioWrapper;
	public:
		XAudioInstance(XAudioAudioWrapper* pAudio, ResPtr resPtr);

		void* GetImplementationData();
		bool Restore();

		bool Play(I32 volumeParam, bool loopParam);
		bool Pause();
		bool Stop();
		bool Resume();

		bool TogglePause();
		bool IsPlaying();
		void SetVolume(I32 volumeParam);
		void SetLooping(bool val);
		F32 GetProgress();
		I32 GetVoiceIndex();
	};
}
#endif