#ifdef WIN32
#include "DSoundAudio.h"
#include "SoundResource.h"
#include "Memory/Allocator.h"
#include "Constants/AllocTypes.h"
#include "Math/MathFunctions.h"

using namespace LeEK;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#endif

HRESULT DSoundAudioWrapper::SetPrimaryBufferFormat(DWORD numPriChannels, 
													DWORD priFrequency, 
													DWORD priBitRate)
{
	//I don't even think anyone uses DirectMusic anymore,
	//but don't use this with it or DM will break

	HRESULT res;
	LPDIRECTSOUNDBUFFER priBuf = NULL;

	//anyway, ensure we have a DSound interface
	if(!dSound)
	{
		return CO_E_NOTINITIALIZED;
	}

	//make our sound buffer
	DSBUFFERDESC bufDesc;
	memset(&bufDesc, 0, sizeof(DSBUFFERDESC));
	bufDesc.dwSize = sizeof(DSBUFFERDESC);
	bufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	bufDesc.dwBufferBytes = 0;	//making a primary buffer, so must be 0
	bufDesc.lpwfxFormat = NULL;	//again, primary buffer, so must be null

	res = dSound->CreateSoundBuffer(&bufDesc, &priBuf, NULL);
	if(FAILED(res))
	{
		return res;
	}

	//and format the buffer
	WAVEFORMATEX wFmt;
	memset(&wFmt, 0, sizeof(WAVEFORMATEX));
	wFmt.wFormatTag			= WAVE_FORMAT_PCM;	//should hold decompressed data
	wFmt.nChannels			= numPriChannels;
	wFmt.nSamplesPerSec		= priFrequency;
	wFmt.wBitsPerSample		= priBitRate;
	//Specifies how large a chunk of sound for the format is.
	//Rather important if we ever end up streaming music
	wFmt.nBlockAlign		= (wFmt.wBitsPerSample / 8) * wFmt.nChannels;
	wFmt.nAvgBytesPerSec	= wFmt.nSamplesPerSec * wFmt.nBlockAlign;

	res = priBuf->SetFormat(&wFmt);
	if(FAILED(res))
	{
		return res;
	}

	SAFE_RELEASE(priBuf);

	return S_OK;
}

DSoundAudioWrapper::DSoundAudioWrapper(const Win32Platform* pPlat)
{
	dSound = NULL;
	plat = pPlat;
}

bool DSoundAudioWrapper::Initialize()
{
	//lazy initialization
	if(initialized)
	{
		return true;
	}
	samples.clear();

	//release any interface to dSound, if it exists
	SAFE_RELEASE(dSound);

	HRESULT res;
	//setup DX interface
	res = DirectSoundCreate8(NULL, &dSound, NULL);
	if(FAILED(res))
	{
		LogE("Failed to create DirectSound device!");
		return false;
	}

	//setup cooperative level
	//we want priority to define our own buffer format
	//without causing trouble for other processes
	res = dSound->SetCooperativeLevel(plat->GetWindowHnd(), DSSCL_PRIORITY);
	if(FAILED(res))
	{
		LogE("Failed to set sound cooperative level!");
		return false;
	}

	//and setup buffer format
	//44.1kHZ @ 16 bit is default, maybe keep 8 channels
	//TODO: make defined by configuration?
	res = SetPrimaryBufferFormat(8, 44100, 16);
	if(FAILED(res))
	{
		LogE("Failed to setup primary sound buffer!");
		return false;
	}

	initialized = true;
	return true;
}

void DSoundAudioWrapper::Shutdown()
{
	//real simple, do interface shutdown
	//and release the DSound hook
	if(initialized)
	{
		AudioWrapper::Shutdown();
		SAFE_RELEASE(dSound);
		initialized = false;
	}
}

ISoundInstance* DSoundAudioWrapper::InitInstance(ResPtr resource)
{
	if(!dSound)
	{
		return NULL;
	}
	if(!resource->Extra() || !strcmp(resource->Extra()->TypeName(), "SoundExtraData"))
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

	LPDIRECTSOUNDBUFFER sampleBuf;
	//build the buffer.
	//we at least need to be able to adjust the volume,
	//3D sound may also be desired but won't be used at the moment.
	//Note that 3D sound requires mono sources!
	DSBUFFERDESC bufDesc;
	memset(&bufDesc, 0, sizeof(DSBUFFERDESC));
	bufDesc.dwSize = sizeof(DSBUFFERDESC);
	bufDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufDesc.dwBufferBytes = resource->Size(); //TODO: verify this still applies.
	bufDesc.guid3DAlgorithm = GUID_NULL;
	bufDesc.lpwfxFormat = NULL;

	HRESULT res = dSound->CreateSoundBuffer(&bufDesc, &sampleBuf, NULL);
	if(FAILED(res))
	{
		LogE("Failed to initialize sound resource!");
		return NULL;
	}

	ISoundInstance* instance = CustomNew<DSoundInstance>(SOUND_ALLOC, "SoundAlloc", sampleBuf, resource);
	samples.push_front(instance);

	return instance;
}

void DSoundAudioWrapper::ReleaseInstance(ISoundInstance* instance)
{
	instance->Stop();
	samples.remove(instance);
}

DSoundInstance::DSoundInstance(LPDIRECTSOUNDBUFFER bufParam, ResPtr resPtr) 
	: SoundInstance(resPtr)
{
	//L_ASSERT(resPtr->Size() > sizeof(SoundResource) && "Sound resource is too small!");
	buf = bufParam;
	fillBuffer();
}

void* DSoundInstance::GetImplementationData()
{
	if(!Restore())
	{
		return NULL;
	}
	return buf;
}

bool DSoundInstance::Play(I32 volumeParam, bool loopParam)
{
	Stop();
	volume = volumeParam;
	looping = loopParam;

	return Resume();
}

bool DSoundInstance::Pause()
{
	//refresh the buffer as needed
	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	if(!dsBuf)
	{
		return false;
	}
	
	paused = true;
	return (dsBuf->Stop() == DS_OK);
}

//kinda like pause, but we reset the buffer's head.
bool DSoundInstance::Stop()
{
	//refresh the buffer as needed
	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	if(!dsBuf)
	{
		return false;
	}
	
	paused = false;
	if(dsBuf->Stop() != DS_OK)
	{
		return false;
	}
	//move buffer head to the start
	return(dsBuf->SetCurrentPosition(0) == DS_OK);
}

bool DSoundInstance::Resume()
{
	paused = false;

	//refresh the buffer as needed
	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	if(!dsBuf)
	{
		return false;
	}

	dsBuf->SetVolume(volume);
	DWORD flags = looping ? DSBPLAY_LOOPING : 0;
	
	return (dsBuf->Play(0, 0, flags) == DS_OK);
}

bool DSoundInstance::TogglePause()
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

bool DSoundInstance::IsPlaying()
{
	DWORD status = 0;
	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	L_ASSERT(dsBuf && "Couldn't get DirectSound buffer!");
	dsBuf->GetStatus(&status);
	bool isPlaying = (status & DSBSTATUS_PLAYING) != 0;
	return isPlaying;
}

void DSoundInstance::SetVolume(I32 volParam)
{
	const I32 minVol = DSBVOLUME_MIN;
	const I32 maxVol = DSBVOLUME_MAX;

	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	if(!dsBuf)
	{
		return;
	}

	I32 clampedVol = Math::Clamp(volParam, 0, 100);

	//DSound uses a linear scale, so remember to apply logarithmic scaling beforehand
	F32 volScale = volume / 100.0f;
	F32 logCoeff = volScale > 0.1f ? (1 + Math::Log10(volScale)) : 0;
	F32 range = maxVol - minVol;
	//and move into DSound's range
	F32 finalVol = (range * logCoeff) + minVol;

	L_ASSERT(finalVol >= minVol && finalVol <= maxVol && "Volume is out of DirectSound range!");
	HRESULT result = dsBuf->SetVolume(finalVol);
	L_ASSERT(result == S_OK && "Failed to set volume!");
}

bool DSoundInstance::Restore()
{
	//need to refill buffer...
	HRESULT result;
	BOOL restored;
	result = restoreBuffer(&restored);
	
	if(FAILED(result))
	{
		return false;
	}

	//was the buffer definitely lost?
	if(restored)
	{
		//now refill the buffer
		result = fillBuffer();
		if(FAILED(result))
		{
			return false;
		}
	}

	//otherwise, we don't need to do any housekeeping, use the buffer as is
	return true;
}

HRESULT DSoundInstance::restoreBuffer(BOOL* wasRestored)
{
	HRESULT result;

	if(!buf)
	{
		return CO_E_NOTINITIALIZED;
	}
	if(wasRestored != NULL)
	{
		*wasRestored = false;
	}
	DWORD status = 0;
	result = buf->GetStatus(&status);
	if(FAILED(result))
	{
		return result;
	}

	if(status & DSBSTATUS_BUFFERLOST)
	{
		//we might not immediately have control;
		//do multiple restores for 1 second
		I32 restoreCount = 0;
		do
		{
			result = buf->Restore();
			++restoreCount;
			if(result == DSERR_BUFFERLOST)
			{
				Sleep(10);
			}
		}
		while(result == DSERR_BUFFERLOST && restoreCount < 100);

		if(wasRestored != NULL)
		{
			*wasRestored = true;
		}

		return S_OK;
	}
	//didn't need to restore anything...
	else
	{
		return S_FALSE;
	}
}

HRESULT DSoundInstance::fillBuffer()
{
	HRESULT result;
	VOID* lockedBuf = NULL;
	//sizes in bytes
	DWORD lockedBufSize = 0;
	DWORD wavDatToRead = 0;

	if(!buf)
	{
		return CO_E_NOTINITIALIZED;
	}

	//ensure we have focus
	result = restoreBuffer(NULL);
	if(FAILED(result))
	{
		return result;
	}

	I32 pcmBufSize = resource->Size();// - sizeof(SoundResource);
	
	//lock the buffer
	result = buf->Lock(	0,				//start of the buffer
						pcmBufSize,		//lock enough for the entire decompressed data
						&lockedBuf,		//stores pointer to locked data
						&lockedBufSize,	//stores size of locked buffer
						NULL,			//don't need a second buffer, so the rest's null
						NULL,
						0);			//and use default flags
	if(FAILED(result))
	{
		return result;
	}
	
	SoundExtraData* soundRes = (SoundExtraData*)resource->Extra();

	if(pcmBufSize == 0)
	{
		//blank, so fill buffer with silence
		BYTE silenceVal = soundRes->GetFormat().BitsPerSample == 8 ? 128 : 0;
		FillMemory((BYTE*)lockedBuf, lockedBufSize, silenceVal);
	}
	else
	{
		//we do have data, prep a copy
		const char* pcmBuf = resource->Buffer();//soundRes->GetPCMBuffer();
		if(!pcmBuf)
		{
			return E_FAIL;
		}
		CopyMemory(lockedBuf, pcmBuf, pcmBufSize);
		//if buffers mismatch, pad the locked buffer with silence
		if(pcmBufSize < (I32)lockedBufSize)
		{
			BYTE silenceVal = soundRes->GetFormat().BitsPerSample == 8 ? 128 : 0;
			FillMemory(	(BYTE*)lockedBuf + pcmBufSize,
						lockedBufSize - pcmBufSize,
						silenceVal);
		}
	}

	//we're done, unlock the buffer
	buf->Unlock(lockedBuf, lockedBufSize, NULL, 0);

	return S_OK;
}

F32 DSoundInstance::GetProgress()
{
	LPDIRECTSOUNDBUFFER dsBuf = (LPDIRECTSOUNDBUFFER)GetImplementationData();
	DWORD headPos = 0;

	dsBuf->GetCurrentPosition(&headPos, NULL);
	F32 len = resource->Size();// - sizeof(SoundResource);
	return (F32)headPos / len;
}
#endif