#include "AudioManager.h"

using namespace LeEK;

AudioWrapper::AudioWrapper()
{
	allPaused = false;
	initialized = false;
}

void AudioWrapper::Shutdown()
{
	//stop and pop off all of the instances
	SoundInstanceList::iterator i = samples.begin();

	while(i != samples.end())
	{
		ISoundInstance* inst = *i;
		inst->Stop();
		++i;
	}
	//since i is always pointing to the front element,
	//this will basically move down to the next element
	//while removing the current element from the list.
	samples.clear();
}

void AudioWrapper::StopAllSounds()
{
	SoundInstanceList::iterator end = samples.end();
	for(SoundInstanceList::iterator i = samples.begin(); i != samples.end(); ++i)
	{
		ISoundInstance* inst = *i;
		inst->Stop();
	}
	//guess this is technically true, isn't it?
	allPaused = false;
}

void AudioWrapper::PauseAllSounds()
{
	SoundInstanceList::iterator end = samples.end();
	for(SoundInstanceList::iterator i = samples.begin(); i != samples.end(); ++i)
	{
		ISoundInstance* inst = *i;
		inst->Pause();
	}
	allPaused = true;
}

void AudioWrapper::ResumeAllSounds()
{
	SoundInstanceList::iterator end = samples.end();
	for(SoundInstanceList::iterator i = samples.begin(); i != samples.end(); ++i)
	{
		ISoundInstance* inst = *i;
		inst->Resume();
	}
	allPaused = false;
}