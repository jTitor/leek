#include "SoundResource.h"

using namespace LeEK;

bool LeEK::operator==(const AudioFormat& lhs, const AudioFormat& rhs)
	{
		return	lhs.NumChannels == rhs.NumChannels &&
				lhs.SamplesPerSec == rhs.SamplesPerSec &&
				lhs.AvgBytesPerSec == rhs.AvgBytesPerSec &&
				lhs.BlockAlignment == rhs.BlockAlignment &&
				lhs.BitsPerSample == rhs.BitsPerSample;
	}
bool LeEK::operator!=(const AudioFormat& lhs, const AudioFormat& rhs)
{
	return !operator==(lhs, rhs);
}