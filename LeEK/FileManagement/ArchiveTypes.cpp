#include "ArchiveTypes.h"
#include "Memory/Allocator.h"
#include "Logging/Log.h"
#include "Constants/AllocTypes.h"
#include "Math/MathFunctions.h"


using namespace LeEK;

bool LeEK::DecompDeflateBuf(char* src, FileSz srcSz, char* dest, FileSz destSz)
{
	//now you have to setup the decompression stream...
	z_stream dcompStream;
	int err;

	memset(&dcompStream, 0, sizeof(z_stream));
	dcompStream.next_in = (Bytef*)src; //input coming from this buffer
	dcompStream.avail_in = (uInt)srcSz; //this many bytes of comp. data coming in
	dcompStream.next_out = (Bytef*)dest; //write to this buffer
	dcompStream.avail_out = destSz; //this many bytes of decomp. data going out
	dcompStream.zalloc = (alloc_func)0; //...no, we're not letting DEFLATE do allocs.
	dcompStream.zfree = (free_func)0;	//freeing is thus also redundant

	//and now we can do the decompression.
	//pass -MAX_WBITS to indicate there's no zlib headers in the data.
	err = inflateInit2(&dcompStream, -MAX_WBITS);
	if(err != Z_OK)
	{
		LogW("Decompression initialization failed!");
		return false;
	}
	//do ACTUAL decompression.
	err = inflate(&dcompStream, Z_FINISH);
	//and cleanup
	inflateEnd(&dcompStream);
	//compression worked if we get Z_STREAM_END,
	//otherwise we have some unknown problem
	if(err != Z_STREAM_END)
	{
		//just quit here
		LogW("Decompression failed!");
		inflateEnd(&dcompStream);
		return false;
	}
	return true;
}

ZipFile::ZipFile()
{
	filePath =Path();
	file = NULL;
	dirBuf = NULL;
	numEntries = 0;
	FileIndexMap = Map<String, I32>();
}

ZipFile::~ZipFile(void)
{
	End();
}

bool ZipFile::Init(const Path& archivePath)
{
	LogV(String("Opening archive ") + archivePath.ToString());
	//open the file, of course
	file = Filesystem::OpenFileReadOnly(archivePath);
	if(!file)
	{
		LogE(String("Couldn't open archive ") + archivePath.ToString() + "!");
		return false;
	}
	filePath = archivePath;

	//now find the directory listing.
	//For now, assume there's no comments, so just seek to the end of the file.
	//Might be off by one?
	file->SeekFromEnd(sizeof(DirHeader));
	//zero out the directory buffer, and read it in.
	//dirBuf = CustomArrayNew<char>(sizeof(DirHeader), RESFILE_ALLOC, "ResFileAlloc");
	DirHeader dirHeader;
	memset(&dirHeader, 0, sizeof(DirHeader));
	U32 res = file->Read((char*)&dirHeader, sizeof(DirHeader));
	if(res != sizeof(DirHeader))
	{
		LogE(String("Couldn't load directory info for ") + archivePath.GetBaseName() + "!");
		return false;
	}

	if(dirHeader.Signature != DirHeader::SIGNATURE)
	{
		LogE(String(archivePath.GetBaseName()) + " appears to be corrupt!");
		return false;
	}
	LogV(String("Opened archive ") + archivePath.GetBaseName());

	//try to process the file records.
	//From the directory header, we should know the offset to the start of file records.
	//also include space at the end for the pointer array used by dirFilePointers.
	dirBuf = CustomArrayNew<char>(	dirHeader.DirSize + dirHeader.NumDirEntriesOnDisk*sizeof(DirFileHeader*),
									RESFILE_ALLOC, "ResFileBufAlloc");
	file->Seek(dirHeader.DirOffset);
	file->Read(dirBuf, dirHeader.DirSize);
	char* bufPos = dirBuf;
	dirFilePointers = (const DirFileHeader**)(dirBuf + dirHeader.DirSize);
	//make a buffer so we can build strings.
	const U32 BUF_LEN = 255;
	char strBuf[BUF_LEN];
	memset(strBuf, 0, BUF_LEN);
	//run through the file headers...
	for(U32 i = 0; i < dirHeader.NumDirEntriesOnDisk; ++i)
	{
		DirFileHeader& fileHdr = *(DirFileHeader*)bufPos;

		//verify the header
		if(fileHdr.Signature != DirFileHeader::SIGNATURE)
		{
			LogW(String(archivePath.GetBaseName()) + ": File " + i + " appears to be corrupt!");
			return false;
		}
		else
		{
			//if it's legit, store the header's pointer
			dirFilePointers[i] = &fileHdr;
			//the filename's past the end of the header;
			//move up to it and convert separators as necessary
			bufPos += sizeof(DirFileHeader);
//#ifdef WIN32
			for(U32 j = 0; j < fileHdr.FNameLen; ++j)
			{
				if(bufPos[j] == '\\')
				{
					bufPos[j] = '/';
				}
			}
//#endif
			//and add the converted filename to the index map
			strncpy(strBuf, bufPos, Math::Min(BUF_LEN, (U32)fileHdr.FNameLen));
			strBuf[fileHdr.FNameLen] = 0;
			FileIndexMap[ToLower(String(strBuf))] = i;
			//skip the rest to go to the next header.
			bufPos += fileHdr.FNameLen + fileHdr.ExtraLen + fileHdr.CommentLen;
		}
	}
	//and set the archive properties
	numEntries = dirHeader.NumDirEntriesOnDisk;
	return true;
}

void ZipFile::End()
{
	LogV("Closing ZIP archive...");
	//get rid of all the stuff!
	CustomArrayDelete(dirBuf);
	dirBuf = NULL;
	numEntries = 0;
	Filesystem::CloseFile(file);
	file = NULL;
	filePath = Path();
}

String ZipFile::GetFilename(I32 fileIndex) const
{
	if(fileIndex < 0 || fileIndex >= numEntries)
	{
		return String("");
	}
	String result(dirFilePointers[fileIndex]->GetFileName());
	result = result.substr( 0, result.length() - 1 );
	return result;
}

FileSz ZipFile::GetFileLen(I32 fileIndex) const
{
	if(fileIndex < 0 || fileIndex >= numEntries)
	{
		return 0;
	}
	return dirFilePointers[fileIndex]->UnCompSize;
}

bool ZipFile::ReadFile(I32 fileIndex, void* fileBuf)
{
	if(fileIndex < 0 || fileIndex >= numEntries)
	{
		return false;
	}
	//Simple - read the whole file in one go.
	//This runs on the same thread as the engine, so don't use this if the file's bigger than like 64MB
	//or everything will noticeably freeze and people will be mad.
	//Anyway! Process is:
	//	* get indexed dir file header
	//	* go to the offset the header specifies
	//	* read that offset as a local header, verify signature
	
	//Get the offset to the local header and read the header.
	//We can't definitely read the file in one read, there may be extra data in the local header.

	//there might be zero-sized files; these typically indicate folders.
	if(dirFilePointers[fileIndex]->UnCompSize == 0)
	{
		LogV(String("File ") + fileIndex + " appears to be a folder, canceling read");
		return true;
	}

	LocalHeader locHdr;
	memset(&locHdr, 0, sizeof(LocalHeader));
	U32 locHdrOffset = dirFilePointers[fileIndex]->LocalHdrOffset;
	file->Seek(locHdrOffset);
	file->Read((char*)&locHdr, sizeof(LocalHeader));
	if(locHdr.Signature != LocalHeader::SIGNATURE)
	{
		LogW(String("Local header for file ") + fileIndex + " appears corrupt!");
		return false;
	}

	//the header should be valid now, prep to read the compressed data
	file->Seek(locHdrOffset + sizeof(LocalHeader) + locHdr.FNameLen + locHdr.ExtraLen);

	//Now we have compressed data.
	//Check the compression type.
	LogV(String("File ") + fileIndex + ", Compression code: " + locHdr.Compression);
	switch(locHdr.Compression)
	{
	case CompressionType::COMP_NONE:
		{
			//if it's uncompressed, copy to the destination.
			file->Read((char*)fileBuf, locHdr.CompSize);
			break;
		}
	case CompressionType::COMP_DEFLATE:
		{
			//if it's DEFLATE, run zlib's DEFLATE already.
			//alloc a temp buffer
			char* compData = CustomArrayNew<char>(locHdr.CompSize, RESFILE_ALLOC, "TempBufAlloc");
			memset(compData, 0, locHdr.CompSize);
			file->Read(compData, locHdr.CompSize);
			//Decompress the file!
			bool result = DecompDeflateBuf(compData, locHdr.CompSize, (char*)fileBuf, locHdr.UnCompSize);
			//now we're done; get rid of the temporary buffer
			CustomArrayDelete(compData);
			return result;
			break;
		}
	default:
		{
			//otherwise, give up :(
			LogW(String("Unrecognized compression on file ") + fileIndex + "!");
			return false;
		}
	}

	return true;
}

ZipStream* ZipFile::GetStream(I32 fileIndex)
{
	if(fileIndex < 0 || fileIndex >= numEntries)
	{
		return NULL;
	}

	DataStream* resStream = Filesystem::OpenFileReadOnly(filePath);
	if(!resStream)
	{
		LogE("Couldn't allocate memory for the stream, aborting!");
		return NULL;
	}
	//Simple - read the whole file in one go.
	//This runs on the same thread as the engine, so don't use this if the file's bigger than like 64MB
	//or everything will noticeably freeze and people will be mad.
	//Anyway! Process is:
	//	* get indexed dir file header
	//	* go to the offset the header specifies
	//	* read that offset as a local header, verify signature
	
	//Get the offset to the local header and read the header.
	//We can't definitely read the file in one read, there may be extra data in the local header.

	//there might be zero-sized files; these typically indicate folders.
	if(dirFilePointers[fileIndex]->UnCompSize == 0)
	{
		LogV(String("File ") + fileIndex + " appears to be a folder, aborting stream creation");
		return NULL;
	}

	LocalHeader locHdr;
	memset(&locHdr, 0, sizeof(LocalHeader));
	U32 locHdrOffset = dirFilePointers[fileIndex]->LocalHdrOffset;
	resStream->Seek(locHdrOffset);
	resStream->Read((char*)&locHdr, sizeof(LocalHeader));
	if(locHdr.Signature != LocalHeader::SIGNATURE)
	{
		LogW(String("Local header for file ") + fileIndex + " appears corrupt! Aborting stream creation!");
		return NULL;
	}

	//the header should be valid now, prep to read the compressed data
	U32 fileStart = locHdrOffset + sizeof(LocalHeader) + locHdr.FNameLen + locHdr.ExtraLen;
	resStream->Seek(fileStart);

	//Now we have compressed data.
	//Check the compression type.
	if(	locHdr.Compression != CompressionType::COMP_NONE &&
		locHdr.Compression != CompressionType::COMP_DEFLATE)
	{
		LogW(String("Unrecognized compression on file ") + fileIndex + ", aborting stream creation!");
		return NULL;
	}

	return LNew(ZipStream, AllocType::RESFILE_ALLOC, "ZipStreamAlloc")(resStream, fileStart, locHdr.CompSize, locHdr.Compression);
}

//streaming functions
//not doing them for now!

bool ZipFile::ReadLargeFile(I32 fileIndex, void* fileBuf, const FileSz bufSz, void (*progressCallback)(I32, bool&))
{
	if(fileIndex < 0 || fileIndex >= numEntries)
	{
		return false;
	}

	//Anyway! Process is:
	//	* get indexed dir file header
	//	* go to the offset the header specifies
	//	* read that offset as a local header, verify signature
	
	//Get the offset to the local header and read the header.
	//We can't definitely read the file in one read, there may be extra data in the local header.

	//there might be zero-sized files; these typically indicate folders.
	if(dirFilePointers[fileIndex]->UnCompSize == 0)
	{
		LogV(String("File ") + fileIndex + " appears to be a folder, canceling read");
		return true;
	}

	LocalHeader locHdr = {0};
	U32 locHdrOffset = dirFilePointers[fileIndex]->LocalHdrOffset;
	//Now we *do* have the local header's position; 
	//we now must read to figure out what to do with the data it represents.
	file->Seek(locHdrOffset);
	file->Read((char*)&locHdr, sizeof(LocalHeader));
	if(locHdr.Signature != LocalHeader::SIGNATURE)
	{
		LogW(String("Local header for file ") + fileIndex + " appears corrupt!");
		return false;
	}

	//the header should be valid now, prep to read the compressed data
	file->Seek(locHdrOffset + sizeof(LocalHeader) + locHdr.FNameLen + locHdr.ExtraLen);

	//Now we have compressed data.
	//Check the compression type.
	LogV(String("File ") + fileIndex + ", Compression code: " + locHdr.Compression);
	switch(locHdr.Compression)
	{
	case CompressionType::COMP_NONE:
		{
			//if it's uncompressed, copy to the destination.
			file->Read((char*)fileBuf, locHdr.CompSize);
			break;
		}
	case CompressionType::COMP_DEFLATE:
		{
			//if it's DEFLATE, run zlib's DEFLATE already.
			//alloc a temp buffer
			char* compData = CustomArrayNew<char>(locHdr.CompSize, RESFILE_ALLOC, "TempBufAlloc");
			memset(compData, 0, locHdr.CompSize);
			file->Read(compData, locHdr.CompSize);

			//now you have to setup the decompression stream...
			z_stream dcompStream;
			int err;

			memset(&dcompStream, 0, sizeof(z_stream));
			dcompStream.next_in = (Bytef*)compData; //input coming from this buffer
			dcompStream.avail_in = (uInt)locHdr.CompSize; //this many bytes of comp. data coming in
			dcompStream.next_out = (Bytef*)fileBuf; //write to this buffer
			dcompStream.avail_out = locHdr.UnCompSize; //this many bytes of decomp. data going out
			dcompStream.zalloc = (alloc_func)0; //...no, we're not letting DEFLATE do allocs.
			dcompStream.zfree = (free_func)0;	//freeing is thus also redundant

			//and now we can do the decompression.
			//pass -MAX_WBITS to indicate there's no zlib headers in the data.
			err = inflateInit2(&dcompStream, -MAX_WBITS);
			if(err != Z_OK)
			{
				LogW(String("Decompression initialization failed on file ") + fileIndex + "!");
				CustomArrayDelete(compData);
				return false;
			}
			//do ACTUAL decompression.
			err = inflate(&dcompStream, Z_FINISH);
			//and cleanup
			inflateEnd(&dcompStream);
			//compression worked if we get Z_STREAM_END,
			//otherwise we have some unknown problem
			if(err != Z_STREAM_END)
			{
				//just quit here
				LogW(String("Decompression failed on file ") + fileIndex + "!");
				inflateEnd(&dcompStream);
				CustomArrayDelete(compData);
				return false;
			}
			//now we're done; get rid of the temporary buffer
			CustomArrayDelete(compData);
			break;
		}
	default:
		{
			//otherwise, give up :(
			LogW(String("Unrecognized compression on file ") + fileIndex + "!");
			return false;
		}
	}

	return true;
}

I32 ZipFile::Find(const String& filePath) const
{
	Map<String, I32>::const_iterator fileIt = FileIndexMap.find(filePath);
	if(fileIt != FileIndexMap.end())
	{
		return fileIt->second;
	}
	return -1;
}

void ZipStream::init(DataStream* pFile, FileSz pStreamStart, FileSz pStreamSz, U16 pCompType)
{
	//Setup the stream.
	memset(&dcompStream, 0, sizeof(z_stream));

	file = pFile;
	streamStart = pStreamStart;
	streamSz = pStreamSz;
	compType = pCompType;
}

ZipStream::ZipStream(DataStream* pFile, FileSz pStreamStart, FileSz pStreamSz, U16 pCompType)
{
	init(pFile, pStreamStart, pStreamSz, pCompType);
}

ZipStream::~ZipStream()
{
	Filesystem::CloseFile(file);
}

FileSz ZipStream::Read(U8* dest, FileSz readSz)
{
	//Clamp to the number of bytes left in the stream.
	FileSz fixedSz = Math::Max(readSz, streamSz - (file->ReadPos() - streamStart));
	switch(compType)
	{
	case CompressionType::COMP_NONE:
		{
			//if it's uncompressed, copy to the destination.
			return file->Read((char*)dest, fixedSz);
		}
	case CompressionType::COMP_DEFLATE:
		{
			//if it's DEFLATE, run zlib's DEFLATE already.
			//alloc a temp buffer
			FileSz startPos = file->ReadPos();
			char* compData = LArrayNew(char, fixedSz, RESFILE_ALLOC, "TempBufAlloc");
			memset(compData, 0, fixedSz);
			file->Read(compData, fixedSz);

			//now you have to setup the decompression stream...
			int err;

			memset(&dcompStream, 0, sizeof(z_stream));
			dcompStream.next_in = (Bytef*)compData; //input coming from this buffer
			dcompStream.avail_in = (uInt)fixedSz; //this many bytes of comp. data coming in
			dcompStream.next_out = (Bytef*)dest; //write to this buffer
			dcompStream.avail_out = fixedSz; //this many bytes of decomp. data going out
			dcompStream.zalloc = (alloc_func)0; //...no, we're not letting DEFLATE do allocs.
			dcompStream.zfree = (free_func)0;	//freeing is thus also redundant

			//and now we can do the decompression.
			//pass -MAX_WBITS to indicate there's no zlib headers in the data.
			err = inflateInit2(&dcompStream, -MAX_WBITS);
			if(err != Z_OK)
			{
				LogW("Decompression initialization failed!");
				LArrayDelete(compData);
				file->Seek(startPos);
				return 0;
			}
			//Do ACTUAL decompression.
			//Write to buffer as we decomp.
			err = inflate(&dcompStream, Z_SYNC_FLUSH);
			//Update the read head according to the number of bytes actually read.
			FileSz actualRead = dcompStream.total_in;
			if(actualRead != fixedSz)
			{
				file->Seek(startPos + actualRead);
			}
			FileSz actualWritten = dcompStream.total_out;
			//and cleanup
			inflateEnd(&dcompStream);
			//compression worked if we get Z_STREAM_END,
			//Z_OK or Z_BUF_ERROR (decompressed data is too big);
			//otherwise we have some unknown problem
			if(	err != Z_STREAM_END &&
				err != Z_OK &&
				err != Z_BUF_ERROR)
			{
				//just quit here
				LogW("Decompression failed!");
				inflateEnd(&dcompStream);
				LArrayDelete(compData);
				file->Seek(startPos);
				return 0;
			}
			//now we're done; get rid of the temporary buffer
			LArrayDelete(compData);
			return actualWritten;
		}
	default:
		{
			//otherwise, give up :(
			L_ASSERT(false && "Created ZipStream with unrecognized compression!");
			LogW("Unrecognized compression!");
			return 0;
		}
	}
}

FileSz ZipStream::Seek(FileSz relPos)
{
	FileSz fixedSeekPos = streamStart + Math::Max(relPos, streamSz);
	return file->Seek(fixedSeekPos);
}