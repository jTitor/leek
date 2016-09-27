#include "SoundLoaders.h"
#include "SoundResource.h"
#include "Logging/Log.h"
#include <libogg/include/ogg/ogg.h>
#include <libogg/include/ogg/os_types.h>
#include <libvorbis/include/vorbis/vorbisenc.h>

//We need to link libraries.
#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")

using namespace LeEK;

typedef unsigned char uChar;

/**
Converts a given four character code to an integer.
*/
#define FOUR_CC(ch0, ch1, ch2, ch3)	((U32)(uChar)(ch0) | ((U32)(uChar)(ch1) << 8) | \
			((U32)(uChar)(ch2) << 16) | ((U32)(uChar)(ch3) << 24 )) 

/**
Reads a 32-bit unsigned int from the given data buffer and read head;
also advances the read head by an int.
*/
U32 readU32(char* dataStart, U32* pos)
{
	U32 res = *(U32*)(dataStart + *pos);
	*pos += sizeof(U32);
	return res;
}

WAVLoader::WAVLoader()
{
	resetFileInfo();
}

/**
Fills a given AudioFormat with wave format data.
*/
void WAVLoader::initAudioFormat(AudioFormat* fmtDst, const WaveFormat& fmtSrc)
{
	fmtDst->NumChannels = fmtSrc.nChannels;
	fmtDst->SamplesPerSec = fmtSrc.nSamplesPerSec;
	fmtDst->AvgBytesPerSec = fmtSrc.nAvgBytesPerSec;
	fmtDst->BlockAlignment = fmtSrc.nBlockAlign;
	fmtDst->BitsPerSample = fmtSrc.wBitsPerSample;
}

void WAVLoader::resetFileInfo()
{
	fileParsed = false;
	fileLoadedSz = 0;
	fileLenMs = 0;
	dataStart = 0;
	memset(&fileFmt, 0, sizeof(WaveFormat));
}

bool WAVLoader::lazyParse(char* wavFile, size_t bufLen)
{
	if(fileParsed)
	{
		return true;
	}
	fileParsed = parseFile(wavFile, bufLen);
	return fileParsed;
}

bool WAVLoader::parseFile(char* wavFile, size_t bufLen)
{
	//Now setup parsing.
	U32 filePos = 0;
	U32 fileEnd = 0;
	U32 length = 0;
	U32 chunkType = 0;
	U32 pos = 0;

	//The wavFile pointer should be pointing to the start of the file;
	//read the first 4 bytes of the file.
	chunkType = readU32(wavFile, &pos);
	//Valid .wav files start with "RIFF"; quit if this isn't there.
	if(chunkType != FOUR_CC('R','I','F','F'))
	{
		LogW("Sound file does not appear to be a valid .wav file!");
		return false;
	}
	
	//If the file is valid, the length is the next thing found.
	length = readU32(wavFile, &pos);
	//Since the file start is 0, the file's end is implicitly the length we just got;
	//reduce it by 4 bytes (to avoid including the initial tag?)
	fileEnd = length - 4;

	//Next, see if the data stored is actually a waveform.
	chunkType = readU32(wavFile, &pos);
	if(chunkType != FOUR_CC('W','A','V','E'))
	{
		LogW("RIFF file does not appear to contain a waveform!");
		return false;
	}

	bool foundFormat = false;
	bool foundData = false;

	//Now we want to copy all of the wave data;
	//there may be format data, which we also have to handle.
	while(filePos < fileEnd)
	{
		//Each chunk will start with its type and the length of the chunk,
		//so read those values in.
		chunkType = readU32(wavFile, &pos);
		//Don't forget to advance the file counter.
		filePos += sizeof(U32);

		length = readU32(wavFile, &pos);
		filePos += sizeof(U32);

		//Now figure out what kind of chunk this is...
		switch(chunkType)
		{
			//Compressed data?
			case FOUR_CC('f','a','c','t'):
			{
				LogW("WAV file contained compressed data; ignoring!");
				break;
			}
			//Format data?
			case FOUR_CC('f','m','t',' '):
			{
				foundFormat = true;
				//Reinterpret the current position in the file as a wave format struct.
				fileFmt = *(WaveFormat*)(wavFile+pos);
				//Save this chunk length to the audio format? Not sure.
				break;
			}
			//Actual sound data?
			case FOUR_CC('d','a','t','a'):
			{
				//Note that we've found the data!
				foundData = true;
				//This raw data can be sent to anything that reads PCM data;
				//note its size and the offset to the data for later loading.
				dataStart = pos;//(size_t)pos - (size_t)wavFile;
				fileLoadedSz = length;
				break;
			}
		}

		//Move the read head past this chunk.
		pos += length;
		//Advance the file counter.
		filePos += length;

		//Once we've found the data buffer, we can also figure out how long the track should be.
		if(foundFormat && foundData)
		{
			fileLenMs = (fileLoadedSz * 1000) / fileFmt.nAvgBytesPerSec;
			//Found all needed data; report success!
			return true;
		}

		//Otherwise, advance to the next chunk.
		//Ensure the read head's aligned.
		if(length & 1)
		{
			++pos;
			++filePos;
		}
	}

	//We read the whole file, but didn't find the required data.
	return false;
}

U32 WAVLoader::GetLoadedResSize(char* rawBuf, U32 rawSize)
{
	if(!lazyParse(rawBuf, rawSize))
	{
		return 0;
	}
	return fileLoadedSz;
}

bool WAVLoader::LoadResource(char* rawBuf, U32 rawSize, std::shared_ptr<Resource> resource)
{
	if(!lazyParse(rawBuf, rawSize))
	{
		return false;
	}

	//First, get the extra data header.
	SoundExtraData* extra = LNew(SoundExtraData, AllocType::SOUND_ALLOC, "SoundAlloc")();
	//Null out the sound data's format struct.
	memset(&extra->fmt, 0, sizeof(AudioFormat));
	extra->sType = SoundType::WAVE;
	//Copy the format data to the sound data's format struct.
	initAudioFormat(&extra->fmt, fileFmt);
	extra->soundLenMs = fileLenMs;
	extra->bufSz = fileLoadedSz;
	//Hook this to the resource.
	resource->SetExtra(extra);
	//Now copy the PCM data.
	memcpy(resource->WriteableBuffer(), rawBuf+dataStart, fileLoadedSz);

	//Reset instance data for the next file.
	resetFileInfo();
	return true;
}

//libogg needs custom functions to mimic the C library's file functions.
//Following are the mimic functions.
size_t VorbisRead(void* dest, size_t elemSz, size_t szToRead, void* src)
{
	//Check that the source is valid.
	if(src == NULL)
	{
		return -1;
	}
	//Nothing says that this will be valid, but hey.
	OggMemoryFile* data = (OggMemoryFile*)src;

	//Prep the read.
	size_t trueSzToRead = szToRead * elemSz;
	size_t maxSzRead = data->DataSz - data->DataRead;
	//Clamp the read request to the number of bytes actually left.
	if(trueSzToRead > maxSzRead)
	{
		trueSzToRead = maxSzRead;
	}

	//Now copy to the destination buffer.
	if(trueSzToRead <= 0)
	{
		return 0;
	}
	memcpy(dest, (char*)data->Data + data->DataRead, trueSzToRead);
	data->DataRead += trueSzToRead;
	return trueSzToRead;
}

int VorbisSeek(void* src, ogg_int64_t offset, int origin)
{
	//Check that the source is valid.
	if(src == NULL)
	{
		return -1;
	}
	OggMemoryFile* data = (OggMemoryFile*)src;
	
	//Where we go depends on what point in the file we're moving relative to.
	//As always, remember to clamp the seek offset to a position within the file.
	ogg_int64_t trueOffset = 0;
	switch(origin)
	{
		//Relative to beginning of file.
	case SEEK_SET:
		{
			trueOffset = (offset < data->DataSz) ? offset : data->DataSz;
			data->DataRead = (size_t)trueOffset;
			break;
		}
		//Relative to the current read position.
	case SEEK_CUR:
		{
			size_t distToEOF = data->DataSz - data->DataRead;
			trueOffset = (offset < distToEOF) ? offset : distToEOF;
			data->DataRead += (size_t)trueOffset;
			break;
		}
		//Relative to the file's end.
	case SEEK_END:
		{
			data->DataRead = data->DataSz + 1;
			break;
		}
	default:
		{
			L_ASSERT(false && "VorbisSeek called with bad \"origin\" parameter; please pass SEEK_SET, SEEK_CUR, or SEEK_END!");
			return -1;
		}
	}

	return 0;
}

int VorbisClose(void* src)
{
	//Since this buffer should be managed by the res. manager,
	//do nothing
	return 0;
}

long VorbisTell(void* src)
{
	//Check that the source is valid.
	if(src == NULL)
	{
		return -1;
	}
	OggMemoryFile* data = (OggMemoryFile*)src;
	return (long)(data->DataRead <= data->DataSz ? data->DataRead : data->DataSz);
}

OGGLoader::OGGLoader()
{
	resetFileInfo();

	//Prep all the libogg stuff.
	//Hook in the callbacks.
	oggCallbacks.read_func = VorbisRead;
	oggCallbacks.close_func = VorbisClose;
	oggCallbacks.seek_func = VorbisSeek;
	oggCallbacks.tell_func = VorbisTell;
}

void OGGLoader::resetFileInfo()
{
	memset(&parsedFile, 0, sizeof(OggVorbis_File));
	memset(&memoryFile, 0, sizeof(OggMemoryFile));
	fileOpened = false;
}

bool OGGLoader::openFile(char* oggFile, size_t bufLen)
{
	memoryFile = OggMemoryFile();
	memoryFile.DataRead = 0;
	memoryFile.DataSz = bufLen;
	memoryFile.Data = (U8*)oggFile;

	int oggResult = ov_open_callbacks(&memoryFile, &parsedFile, NULL, 0, oggCallbacks);
	fileOpened = oggResult >= 0;

	return fileOpened;
}

bool OGGLoader::lazyOpen(char* oggFile, size_t bufLen)
{
	if(fileOpened)
	{
		return true;
	}
	return openFile(oggFile, bufLen);
}

bool OGGLoader::parseFile(char* oggFile, size_t bufLen, std::shared_ptr<Resource> resPtr)
{
	if(!lazyOpen(oggFile, bufLen))
	{
		L_ASSERT(false && "Failed to initialize OGG parser!");
		return false;
	}

	//Get data on the audio format of the file.
	vorbis_info* fileInf = ov_info(&parsedFile, -1);

	//Get the extra data header.
	SoundExtraData* extra = LNew(SoundExtraData, AllocType::SOUND_ALLOC, "SoundAlloc")();

	const U32 OGGBitsPerSample = 16;

	extra->sType = SoundType::OGG;
	extra->fmt.NumChannels = fileInf->channels;
	extra->fmt.BitsPerSample = OGGBitsPerSample;
	extra->fmt.SamplesPerSec = fileInf->rate;
	extra->fmt.AvgBytesPerSec = extra->fmt.SamplesPerSec * extra->fmt.NumChannels * 2;
	extra->fmt.BlockAlignment = 2 * extra->fmt.NumChannels;

	//Calculate the size... again.
	U32 size = ov_pcm_total(&parsedFile, -1);
	//This gets us the number of samples per channel, so also multiply by channel count.
	size *= 2 * fileInf->channels;

	if(size != resPtr->Size())
	{
		LogW("OGG file's decompressed size does not match the resource's buffer size!");
		return false;
	}
	extra->bufSz = size;

	//Decompress the entire file.
	U32 sampleSz = 4096 * OGGBitsPerSample;
	U32 pos = 0;
	int bitStreamNum = 0;
	int bytesRead = 1;

	while(bytesRead > 0 && pos < size)
	{
		bytesRead = ov_read(&parsedFile, resPtr->WriteableBuffer() + pos, sampleSz,
							0, 2, 1, &bitStreamNum);
		pos += bytesRead;
		//Clamp the amount read if the buffer has less than the
		//default sample size left.
		if(size - pos < sampleSz)
		{
			sampleSz = size - pos;
		}
	}

	//File has been decoded, save the last piece of info.
	extra->soundLenMs = 1000.0f * ov_time_total(&parsedFile, -1);
	//Hook it to the resource, too.
	resPtr->SetExtra(extra);

	return true;
}

U32 OGGLoader::GetLoadedResSize(char* rawBuf, U32 rawSize)
{
	if(!lazyOpen(rawBuf, rawSize))
	{
		L_ASSERT(false && "Failed to initialize OGG parser!");
		return 0;
	}
	vorbis_info* fileInf = ov_info(&parsedFile, -1);
	if(!fileInf)
	{
		L_ASSERT(false && "Couldn't get OGG file info!");
		return 0;
	}
	auto result = ov_pcm_total(&parsedFile, -1);
	if(result == OV_EINVAL)
	{
		L_ASSERT(false && "Couldn't get OGG data size!");
		return 0;
	}
	//This gets us the number of samples per channel, so also multiply by channel count.
	result *= 2 * fileInf->channels;
	return (U32)result;
}

bool OGGLoader::LoadResource(char* rawBuf, U32 rawSize, std::shared_ptr<Resource> resource)
{
	if(!lazyOpen(rawBuf, rawSize))
	{
		L_ASSERT(false && "Failed to initialize OGG parser!");
		return 0;
	}
	bool result = parseFile(rawBuf, rawSize, resource);
	//Whatever result we got, we always need to clear the vorbis file.
	ov_clear(&parsedFile);
	resetFileInfo();
	return result;
}

//Things to test streaming with.
void streamOgg()
{
	//setup callbacks
	ov_callbacks oggCbk = {0};
	oggCbk.read_func = VorbisRead;
	oggCbk.close_func = VorbisClose;
	oggCbk.seek_func = VorbisSeek;
	oggCbk.tell_func = VorbisTell;

	//get the file open
	
}