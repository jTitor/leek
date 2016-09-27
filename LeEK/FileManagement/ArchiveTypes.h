#pragma once
#include "Datatypes.h"
#include "FileManagement/Filesystem.h"
#include "FileManagement/DataStream.h"
#include "DataStructures/STLContainers.h"
#include "Libraries/Zlib/zlib.h"

namespace LeEK
{
	enum CompressionType
	{
		//compression methods
		COMP_NONE = 0,
		COMP_DEFLATE = 8
	};

	//Decompresses a full chunk of compressed data
	//to an output buffer.
	bool DecompDeflateBuf(char* src, FileSz srcSz, char* dest, FileSz destSz);
	//FileSz DecompDeflateChunk(char* chunk

	class ZipStream;

	class ZipFile
	{
	private:
		struct DirHeader;
		struct DirFileHeader;
		struct LocalHeader;
		Path filePath;
		DataStream* file;
		//buffer for archive directory struct
		char* dirBuf;
		//points to individual entries in dirBuf.
		//presumably dir. file headers aren't equally spaced apart?
		const DirFileHeader** dirFilePointers;
		I32 numEntries;
	public:
		ZipFile();
		~ZipFile(void);

		bool Init(const Path& archivePath);
		void End();

		inline I32 GetNumFiles() const { return numEntries; }
		String GetFilename(I32 fileIndex) const;
		FileSz GetFileLen(I32 fileIndex) const;
		bool ReadFile(I32 fileIndex, void* fileBuf);
		//gets index of given file.
		I32 Find(const String& filePath) const;
		ZipStream* GetStream(I32 fileIndex);

		//TODO:
		//should simply load in a different thread and call the callback after each load.
		bool ReadLargeFile(I32 fileIndex, void* fileBuf, const FileSz bufSz, void (*progressCallback)(I32, bool&));

		Map<String, I32> FileIndexMap;
	};
	
	class ZipStream
	{
	private:
		z_stream dcompStream;
		DataStream* file;
		FileSz streamStart;
		FileSz streamSz;
		U16 compType;
		void init(DataStream* pFile, FileSz pStreamStart, FileSz pStreamSz, U16 pCompType);
	public:
		//ZipStream();
		ZipStream(DataStream* pFile, FileSz pStreamStart, FileSz pStreamSz, U16 pCompType);
		~ZipStream();
		/**
		Reads the specified number of bytes into the destination buffer.
		@return the number of bytes read to the destination buffer.
		Can be less than the specified size if the read head is too close to
		the end of the stream.
		*/
		FileSz Read(U8* dest, FileSz readSz);
		/**
		Moves to the specified <em>compressed</em> position in the stream.
		@return the new position of the read head in the stream, relative
		to the start of the stream.
		*/
		FileSz Seek(FileSz relPos);
	};

	//now let's define these headers!
	//they represent file data, so ensure their defined packing is followed
#pragma pack(1)
	struct ZipFile::DirHeader
	{
		//verification for struct
		enum { SIGNATURE = 0x06054b50 };
		U32 Signature;
		//entries for multipart archives.
		//(hopefully) not useful for us, but need to include them
		//anyway for proper reading of the header.
		U16 DiskNum;
		U16 DiskStart;
		//honestly, these two should match.
		//use numDirEntriesOnDisk to be safe, though.
		U16 NumDirEntriesOnDisk;
		U16 TotalDirEntries;
		//size/offset of directory in bytes. Archives can't be larger than 4GB?
		U32 DirSize;
		U32 DirOffset;
		//there can be comments after this main header!
		//It really sucks if there are, since we're going to assume there aren't
		//and our loading code will break if there is.
		U16 CommentLen;
	};
	struct ZipFile::DirFileHeader
	{
		enum
		{	
			SIGNATURE = 0x02014b50,
		};

		U32 Signature;
		U16 VersionMade;
		U16 VersionNeeded;
		//no context to this; meaning depends on compression algorithm.
		U16 Flag;
		//Specifies what algorithm was used to compress the data.
		//See PKWare Zip File Specification, section 4.4.5 for details.
		//What matters is, match against COMP_* enums.
		U16 Compression;
		//not very accurate, since timezone's not specified.
		U16 ModifiedTime;
		U16 ModifiedDate;
		U32 Crc32;
		U32 CompSize;
		U32 UnCompSize;
		//filename immediately follows header
		U16 FNameLen;
		//extra data follows the filename.
		//more useful if this is a ZIP64 file,
		//as the local header offset might be located in that data.
		U16 ExtraLen;
		//and comments follow extra data!
		U16 CommentLen;
		U16 DiskStart;
		//indicates properties of file.
		//apparently, if lowest bit's not set,
		//file's binary.
		U16 IntAttribs;
		//platform dependent stuff!
		U32 ExtAttribs;
		//if this is 0xFFFFFFFF and archive's ZIP64,
		//check extra data for actual offset.
		U32 LocalHdrOffset;

		//I hate pointer arithmetic
		//just remember that for pointers, 1 = size of pointer
		char* GetFileName() const { return (char*)(this + 1); }
		char* GetExtra() const { return GetFileName() + FNameLen; }
		char* GetComment() const { return GetExtra() + ExtraLen; }
	};
	struct ZipFile::LocalHeader
	{
		enum 
		{	
			SIGNATURE = 0x04034b50
		};
		U32 Signature;
		U16 Version;
		U16 Flag;
		U16 Compression;
		U16 ModifiedTime;
		U16 ModifiedDate;
		U32 Crc32;
		U32 CompSize;
		U32 UnCompSize;
		U16 FNameLen;
		U16 ExtraLen;
	};
#pragma pack()
}