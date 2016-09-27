#ifdef WIN32
#include "XAudio2Audio.h"
#include "SoundResource.h"
#include "Memory/Allocator.h"
#include "Constants/AllocTypes.h"
#include "Math/MathFunctions.h"
#include "Win32AudioHelpers.h"
#include "Platforms/Win32Helpers.h"

using namespace LeEK;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#endif

#pragma region XAudioCallbacks
XAudioCallbacks::XAudioCallbacks(XAudioAudioWrapper* pWrapper)
{
	L_ASSERT(pWrapper && "Improperly initialized XAudioCallbacks!");
	wrapper = pWrapper;
}

void XAudioCallbacks::OnBufferStart(void *pBufferContext) {}

void XAudioCallbacks::OnBufferEnd(void *pBufferContext)
{
	if(!pBufferContext)
	{
		return;
	}
	//Check that this isn't supposed to loop.
	XAudioInstance* instance = (XAudioInstance*)pBufferContext;
	if(instance->IsLooping())
	{
		return;
	}
	//Otherwise, notify owning wrapper that a voice has been released.
	wrapper->Stop(instance);
}

void XAudioCallbacks::OnLoopEnd(void *pBufferContext)
{
	if(!pBufferContext)
	{
		return;
	}
	XAudioInstance* instance = (XAudioInstance*)pBufferContext;
	wrapper->Stop(instance);
}

void XAudioCallbacks::OnVoiceError(void *pBufferContext, HRESULT error)
{
	I32 voiceIdx = -1;
	if(pBufferContext)
	{
		XAudioInstance* instance = (XAudioInstance*)pBufferContext;
		voiceIdx = instance->GetVoiceIndex();
	}
	LogW(String("XAudio: voice ") + voiceIdx + " had an error:");
	Win32Helpers::LogError(error);
}

void XAudioCallbacks::OnCriticalError(HRESULT error)
{
	LogE("XAudio crashed! Error is: ");
	Win32Helpers::LogError(error);
}


#pragma endregion

#pragma region XAudioAudioWrapper
XAudioAudioWrapper::XAudioAudioWrapper(AudioFormat pDefAudioFmt) :
	soundQueue(), freeVoiceQueue(), callbacks(this)
{
	engine = NULL;
	outputVoice = NULL;
	for(U32 i = 0; i < MAX_VOICES; ++i)
	{
		inputVoices[i] = NULL;
		freeVoiceQueue.push_back(i);
	}

	defAudioFmt = pDefAudioFmt;
}

bool XAudioAudioWrapper::Initialize()
{
	//lazy initialization
	if(initialized)
	{
		return true;
	}
	samples.clear();

	//release any interface to dSound, if it exists
	SAFE_RELEASE(engine);
	
	//notify COM that we're loading XAudio.
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT res;
	//setup XAudio
	res = XAudio2Create(&engine);
	if(FAILED(res))
	{
		LogE("Failed to create XAudio device!");
		//Any time initialization fails, you also need to shut down COM.
		CoUninitialize();
		return false;
	}

	res = engine->RegisterForCallbacks(&callbacks);
	if(FAILED(res))
	{
		LogE("Failed to attach engine callbacks!");
		//also shutdown the engine, since it's now been initialized
		SAFE_RELEASE(engine);
		CoUninitialize();
		return false;
	}

	//setup mastering (output) voice.
	//we don't need to set anything special for now
	res = engine->CreateMasteringVoice(&outputVoice);
	if(FAILED(res))
	{
		LogE("Failed to create audio output link!");
		//also shutdown the engine, since it's now been initialized
		SAFE_RELEASE(engine);
		CoUninitialize();
		return false;
	}
	auto fmt = ToWaveFmtEx(defAudioFmt);
	//also make all the input voices.
	for(int i = 0; i < MAX_VOICES; ++i)
	{
		res = engine->CreateSourceVoice(&inputVoices[i], &fmt, 0,
										XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks,
										NULL, NULL);
		if(FAILED(res))
		{
			L_ASSERT(false && "Failed to initialize source voice for audio engine!");
		}
		//Activate the voice so it plays sounds on request.
		inputVoices[i]->Start();
	}

	//we're ready!
	initialized = true;
	return true;
}

void XAudioAudioWrapper::Shutdown()
{
	//real simple, do interface shutdown
	//and release the engine hook
	if(initialized)
	{
		AudioWrapper::Shutdown();
		for(int i = 0; i < MAX_VOICES; ++i)
		{
			inputVoices[i]->Stop();
		}
		SAFE_RELEASE(engine);
		CoUninitialize();
		initialized = false;
	}
}

ISoundInstance* XAudioAudioWrapper::InitInstance(ResPtr resource)
{
	if(!engine)
	{
		return NULL;
	}
	if(!resource->Extra() || strcmp(resource->Extra()->TypeName(), "SoundExtraData") != 0)
	{
		LogW("Sound resource is missing metadata, can't play!");
		return NULL;
	}

	SoundExtraData* soundRes = (SoundExtraData*)resource->Extra();
	switch(soundRes->GetSoundType())
	{
		//support .wav and .ogg
	case WAVE:
	case OGG:
		break;
		//don't support .mp3 or .midi
	case MP3:
	case MIDI:
		LogE(".mp3 and .midi are not currently supported!");
		return NULL;
	default:
		LogE("Unknown sound type!");
		return NULL;
	}

	//Check that the format matches. If it doesn't,
	//get very mad
	if(soundRes->GetFormat() != defAudioFmt)
	{
		LogE("Audio engine given mismatched audio file, returning null!");
		return NULL;
	}

	ISoundInstance* instance = CustomNew<XAudioInstance>(SOUND_ALLOC, "SoundAlloc", this, resource);
	instance->SetVolume(1);
	samples.push_front(instance);

	return instance;
}

void XAudioAudioWrapper::ReleaseInstance(ISoundInstance* instance)
{
	instance->Stop();
	samples.remove(instance);
}

I32 XAudioAudioWrapper::nextFreeVoice()
{
	if(freeVoiceQueue.empty())
	{
		return -1;
	}
	U32 idx = freeVoiceQueue.front();
	freeVoiceQueue.pop_front();
	return idx;
}

void XAudioAudioWrapper::markVoiceFree(I32 voiceIdx)
{
	if(voiceIdx < 0 || voiceIdx > MAX_VOICES)
	{
		L_ASSERT(false && "Passing bad voice index to audio engine!");
		return;
	}

	freeVoiceQueue.push_back(voiceIdx);
}

void XAudioAudioWrapper::updateSoundQueue()
{
	if(!soundQueue.empty())
	{
		auto nextInstance = soundQueue.front();
		soundQueue.pop_front();
		nextInstance->Play();
	}
}

bool XAudioAudioWrapper::tryPlay(XAudioInstance* instance)
{
	L_ASSERT(	instance->voiceIdx >= 0 &&
				"Cannot play sound; sound instance is not connected to audio engine!");

	//Check that the voice this instance is connected to isn't already playing.
	if(IsPlaying(instance))
	{
		return false;
	}
	//Otherwise, the voice is ready. Submit the buffer.
	inputVoices[instance->voiceIdx]->SubmitSourceBuffer(&instance->buffer);
	return true;
}

void XAudioAudioWrapper::xAudio2OnBufferFinish()
{
	//Is this sound a looping sound?
	//don't care then.

	//Otherwise, pull out the voice index item
	//and notify the audio wrapper that a voice just became free.
}

bool XAudioAudioWrapper::Play(XAudioInstance* instance)
{
	//Can we get a voice to play this?
	auto voice = nextFreeVoice();
	//If not, quit...
	if(voice < 0)
	{
		//Remember to record that someone wanted to play a sound, though.
		soundQueue.push_back(instance);
		return false;
	}

	//Otherwise, mark the voice as the new voice for the instance.
	instance->voiceIdx = voice;
	//TODO: note that XAUDIO2_BUFFERs have a storage space pContext
	//for whatever small value you want to get on a callback.
	//Stick the voice index in there so the voice can be put on the free queue
	//when the sound's finished playing.
	instance->buffer.pContext = (void*)instance;

	//Now play the voice!
	//If we can't play, record the request and quit.
	if(!tryPlay(instance))
	{
		soundQueue.push_back(instance);
		return false;
	}
	return true;
}

bool XAudioAudioWrapper::Pause(XAudioInstance* instance)
{
	//Don't bother if the instance is already paused,
	//or if it doesn't have an associated voice
	if(instance->IsPaused())
	{
		return true;
	}
	if(instance->voiceIdx < 0)
	{
		return false;
	}
	//The voice will resume where it is when Resume() is called.
	inputVoices[instance->voiceIdx]->Stop();
	return true;
}

bool XAudioAudioWrapper::Resume(XAudioInstance* instance)
{
	//Don't bother if the instance isn't paused,
	//or if it doesn't have an associated voice
	if(!instance->IsPaused())
	{
		return true;
	}
	if(instance->voiceIdx < 0)
	{
		return false;
	}
	//Presumably this voice was previously playing;
	//it will resume where it left off.
	inputVoices[instance->voiceIdx]->Start();
	return true;
}

bool XAudioAudioWrapper::Stop(XAudioInstance* instance)
{
	//Ensure the instance has a valid link to a voice.
	if(instance->voiceIdx < 0)
	{
		return false;
	}
	auto voice = inputVoices[instance->voiceIdx];
	//ACTUALLY stop the voice;
	//we halt output and clear the voice's buffer.
	voice->Stop();
	if(FAILED(inputVoices[instance->voiceIdx]->FlushSourceBuffers()))
	{
		return false;
	}
	//And restart the voice so it's ready for another Play() call.
	voice->Start();

	//If the full stop was successful, note that a voice just became available!
	freeVoiceQueue.push_back(instance->voiceIdx);
	//Also unlink the instance from the voice index.
	instance->voiceIdx = -1;

	//Play any sounds that might be waiting.
	updateSoundQueue();

	return true;
}

bool XAudioAudioWrapper::Restart(XAudioInstance* instance)
{
	//Ensure the instance has a valid link to a voice.
	if(instance->voiceIdx < 0)
	{
		return false;
	}
	auto voice = inputVoices[instance->voiceIdx];
	//ACTUALLY stop the voice;
	//we halt output and clear the voice's buffer.
	voice->Stop();
	if(FAILED(inputVoices[instance->voiceIdx]->FlushSourceBuffers()))
	{
		return false;
	}
	//And restart the voice so it's ready for another Play() call.
	voice->Start();

	//Now play the voice!
	//If we can't play, record the request and quit.
	if(!tryPlay(instance))
	{
		soundQueue.push_back(instance);
		return false;
	}
	return true;
}

bool XAudioAudioWrapper::IsPlaying(XAudioInstance* instance)
{
	//Check that the instance even has a voice.
	if(instance->voiceIdx < 0)
	{
		return false;
	}

	auto voice = inputVoices[instance->voiceIdx];
	XAUDIO2_VOICE_STATE state;
	voice->GetState(&state);
	//If any buffers are already queued, this voice is playing.
	return state.pCurrentBufferContext > 0;
}

void XAudioAudioWrapper::SetVolume(XAudioInstance* instance)
{
	//Ensure the instance has a valid link to a voice.
	if(instance->voiceIdx < 0)
	{
		return;
	}

	//There's multiple channels; you'll have to set them all
	//inputVoices[instance->voiceIdx]->;
	//TODO
}

F32 XAudioAudioWrapper::GetProgress(XAudioInstance* instance)
{
	//6/6: Apparently the XAUDIO2_BUFFER in the instance gets its PlayBegin parameter set here.

	//Ensure the instance has a valid link to a voice.
	if(instance->voiceIdx < 0)
	{
		return -1;
	}
	XAUDIO2_VOICE_STATE vState;
	memset(&vState, 0, sizeof(XAUDIO2_VOICE_STATE));
	inputVoices[instance->voiceIdx]->GetState(&vState);
	//TODO
	return -1;
}
#pragma endregion

#pragma region XAudioInstance
XAudioInstance::XAudioInstance(XAudioAudioWrapper* pAudio, ResPtr resPtr) 
	: SoundInstance(resPtr), audio(pAudio)
{
	L_ASSERT(pAudio && "No audio engine assigned to an audio instance!");
	voiceIdx = -1;
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	if(resPtr->Extra())
	{
		SoundExtraData* sDat = (SoundExtraData*)resPtr->Extra();
		buffer.pAudioData = (const BYTE*)resPtr->Buffer();
		buffer.AudioBytes = sDat->GetBufferSize();
	}
	else
	{
		L_ASSERT(false && "XAudioInstance: Assigned resource doesn't have sound metadata!");
	}
}

void* XAudioInstance::GetImplementationData()
{
	return NULL;
}

bool XAudioInstance::Play(I32 volumeParam, bool loopParam)
{
	volume = volumeParam;
	looping = loopParam;

	if(voiceIdx < 0)
	{
		return audio->Play(this);
	}

	return audio->Restart(this);
}

bool XAudioInstance::Pause()
{	
	paused = true;
	return audio->Pause(this);
}

//kinda like pause, but we reset the buffer's head.
bool XAudioInstance::Stop()
{
	paused = false;
	return audio->Stop(this);
}

bool XAudioInstance::Resume()
{
	paused = false;

	audio->SetVolume(this);
	
	return audio->Resume(this);
}

bool XAudioInstance::TogglePause()
{
	if(paused)
	{
		Resume();
	}
	else
	{
		Pause();
	}
	return true;
}

bool XAudioInstance::IsPlaying()
{
	return audio->IsPlaying(this);
}

//TODO: refactor to take a float.
//XAudio2 volume values are floats!
//Arguably they're even in a 0-1.0 range.
void XAudioInstance::SetVolume(I32 volParam)
{
	//Change these to XAudio constants.
	const I32 minVol = -1;//DSBVOLUME_MIN;
	const I32 maxVol = -1;//DSBVOLUME_MAX;

	I32 clampedVol = Math::Clamp(volParam, 0, 100);
	volume = clampedVol;

	//DSound uses a linear scale, so remember to apply logarithmic scaling beforehand
	F32 volScale = volume / 100.0f;
	F32 logCoeff = volScale > 0.1f ? (1 + Math::Log10(volScale)) : 0;
	F32 range = maxVol - minVol;
	//and move into DSound's range
	F32 finalVol = (range * logCoeff) + minVol;

	L_ASSERT(finalVol >= minVol && finalVol <= maxVol && "Volume is out of DirectSound range!");
	audio->SetVolume(this);
	//HRESULT result = dsBuf->SetVolume(finalVol);
	//L_ASSERT(result == S_OK && "Failed to set volume!");
}

bool XAudioInstance::Restore()
{
	return true;
}

void XAudioInstance::SetLooping(bool val)
{
	SoundInstance::SetLooping(val);
	//update the buffer tag.
	if(val)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else
	{
		buffer.LoopCount = 0;
	}
}

F32 XAudioInstance::GetProgress()
{
	return audio->GetProgress(this);
}

I32 XAudioInstance::GetVoiceIndex()
{
	return voiceIdx;
}
#pragma endregion
#endif