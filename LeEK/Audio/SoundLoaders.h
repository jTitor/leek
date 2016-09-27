#pragma once
#include "ResourceManagement/IResourceLoader.h"
#include "SoundResource.h"
#include <libvorbis/include/vorbis/vorbisfile.h>

namespace LeEK
{
	/**
	Loads .wav files.
	*/
	class WAVLoader : public IResourceLoader
	{
	private:
		//Identical to Microsoft's WAVEFORMATEX
		//(and the specification for fmt chunks at https://ccrma.stanford.edu/courses/422/projects/WaveFormat/);
		//not simply including the struct for platform-independence.
		//Specify no packing so that this matches what's written to disk.
#pragma pack(1)
		typedef struct
		{
		  U16  wFormatTag;
		  U16  nChannels;
		  U32 nSamplesPerSec;
		  U32 nAvgBytesPerSec;
		  U16  nBlockAlign;
		  U16  wBitsPerSample;
		  U16  cbSize;
		} WaveFormat;
#pragma pack()

		WaveFormat fileFmt;
		size_t dataStart;
		U32 fileLoadedSz;
		U32 fileLenMs;
		bool fileParsed;

		/**
		Fills a given AudioFormat with wave format data.
		*/
		void initAudioFormat(AudioFormat* fmtDst, const WaveFormat& fmtSrc);
		void resetFileInfo();
		bool parseFile(char* wavFile, size_t bufLen);
		/**
		Attempts to parse a file if it hasn't already been parsed.
		Returns true if the file has been parsed at any given point or if fileParsed is true;
		Because of this, it's important to call resetFileInfo() before or after loading a new file.
		*/
		bool lazyParse(char* wavFile, size_t bufLen);
	public:
		WAVLoader();
		String GetPattern() { return "*.wav"; }
		//File must be parsed.
		bool UseRawResource() { return false; }
		U32 GetLoadedResSize(char* rawBuf, U32 rawSize);
		bool LoadResource(char* rawBuf, U32 rawSize, std::shared_ptr<Resource> resource);
	};

	//Keeps track of the memory buffer that libogg's using.
	struct OggMemoryFile
	{
		U8* Data;
		size_t DataSz;
		size_t DataRead; //Number of bytes read so far.

		OggMemoryFile()
		{
			Data = NULL;
			DataSz = 0;
			DataRead = 0;
		}
	};

	/**
	Loads .ogg files.
	*/
	class OGGLoader : public IResourceLoader
	{
	private:
		OggVorbis_File parsedFile;
		OggMemoryFile memoryFile;
		ov_callbacks oggCallbacks;
		bool fileOpened;

		void resetFileInfo();
		bool openFile(char* oggFile, size_t bufLen);
		bool lazyOpen(char* oggFile, size_t bufLen);
		bool parseFile(char* oggFile, size_t bufLen, std::shared_ptr<Resource> resPtr);
	public:
		OGGLoader();
		String GetPattern() { return "*.ogg"; }
		//File must be parsed.
		bool UseRawResource() { return false; }
		U32 GetLoadedResSize(char* rawBuf, U32 rawSize);
		bool LoadResource(char* rawBuf, U32 rawSize, std::shared_ptr<Resource> resource);
		bool LoadStreamResource(char* rawBuf, FileSz rawSize, std::shared_ptr<StreamingResource> resource);
	};
}
