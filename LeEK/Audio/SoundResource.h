#pragma once
#include "Datatypes.h"
#include "ResourceManagement/Resource.h"
#include <libvorbis/include/vorbis/vorbisfile.h>

namespace LeEK
{
	enum SoundType { OGG, WAVE, MP3, MIDI, STYPE_END };

	struct AudioFormat
	{
		U16 NumChannels;
		U32 SamplesPerSec;
		U32 AvgBytesPerSec;
		U16 BlockAlignment;
		U16 BitsPerSample;
	};

	class SoundExtraData : ExtraData
	{
		//Loaders need to be able to access this data.
		friend class WAVLoader;
		friend class OGGLoader;
	protected:
		SoundType sType;
		AudioFormat fmt;
		//char* bufStart;
		U32 soundLenMs;
		U32 bufSz;
	public:
		/*
		SoundExtraData(SoundType typeParam, char* bufStartParam, AudioFormat& fmtParam, U32 pSoundLenMs)
		{
			sType = typeParam;
			//bufStart = bufStartParam;
			fmt = fmtParam;
			soundLenMs = pSoundLenMs;
		}
		*/
		SoundExtraData()
		{
			sType = STYPE_END;
			//bufStart = NULL;
			memset(&fmt, 0, sizeof(AudioFormat));
			soundLenMs = 0;
			bufSz = 0;
		}
		~SoundExtraData(void) {}

		SoundType GetSoundType() { return sType; }
		const AudioFormat& GetFormat() const { return fmt; }
		//char* GetRawBuffer() { return bufStart; }
		/**
		Gets the length of the sound, in miliseconds.
		*/
		U32 GetSoundLenMs() { return soundLenMs; }
		U32 GetBufferSize() { return bufSz; }
		const char* TypeName() { return "SoundExtraData"; }
	};

	class OGGExtraData : SoundExtraData
	{
		friend class OGGLoader;
	private:
		OggVorbis_File vorbisInterface;
	public:
		OGGExtraData() : SoundExtraData()
		{
			sType = OGG;
			memset(&vorbisInterface, 0, sizeof(OggVorbis_File));
		}
	};

	class CompressedSndResource
	{
	private:
		//true if the resource has been previously decompressed.
		bool decompressed;
	};

	bool operator==(const AudioFormat& lhs, const AudioFormat& rhs);
	bool operator!=(const AudioFormat& lhs, const AudioFormat& rhs);
}
