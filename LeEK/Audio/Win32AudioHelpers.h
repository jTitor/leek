#pragma once
#ifdef WIN32
#include <Windows.h>
#include "SoundResource.h"

namespace LeEK
{
	WAVEFORMATEX ToWaveFmtEx(AudioFormat aFmt)
	{
		WAVEFORMATEX result;
		memset(&result, 0, sizeof(WAVEFORMATEX));
		//Although the actual stored resource might not be PCM,
		//it will always eventually be output as such.
		result.wFormatTag = WAVE_FORMAT_PCM;
		result.nChannels = aFmt.NumChannels;
		result.nSamplesPerSec = aFmt.SamplesPerSec;
		result.nAvgBytesPerSec = aFmt.AvgBytesPerSec;
		result.nBlockAlign = aFmt.BlockAlignment;
		result.wBitsPerSample = aFmt.BitsPerSample;
		//There should be no extra data needed for this resource.
		result.cbSize = 0;

		return result;
	}
}

#endif