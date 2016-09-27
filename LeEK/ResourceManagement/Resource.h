#pragma once
#include "Strings/String.h"
#include "Datatypes.h"
#include "Memory/Handle.h"

namespace LeEK
{
	class ResourceManager;

	class ResGUID
	{
	private:
		static const U32 MAX_ARCHIVE_NAME = 128;
		static const U16 MAX_NAME_LEN = 256;
		static const char EXT_SEPARATOR = '.';
		static const char DIR_SEPARATOR = '/';
		static const char ARCHIVE_SEPARATOR = ':';
		U32 archSepPos;
		U32 extPos;

		void init(const String& name);
	public:
		char Name[MAX_NAME_LEN];
		//properties
		bool Empty() const { return Name[0] == 0; }
		U32 Length() const;

		ResGUID(const char* name);
		ResGUID(const String& archName, const String& resName);
		//Creates an empty GUID.
		ResGUID();
		/**
		Gets the name of the resource's archive, without the path.
		*/
		String ResArchiveName() const;
		/**
		Gets the name of the file, without the path.
		*/
		String Filename() const;
		/**
		Gets filename without the trailing extension.
		*/
		String BaseName() const;
		/**
		Gets the path of the resource within the archive.
		*/
		const char* ResName() const { return Name + archSepPos + 1; }
		/**
		Gets the extension of the resource.
		*/
		const char* Extension() const { return Name + extPos + 1; }
		bool IsArchiveGUID();
		bool IsRelFileSysGUID();
		bool IsAbsFileSysGUID();
	};

	inline bool operator== (const ResGUID& lhs, const ResGUID& rhs) { return strcmp(lhs.Name, rhs.Name) == 0; }
	inline bool operator!= (const ResGUID& lhs, const ResGUID& rhs) { return !(lhs == rhs); }

	class ExtraData
	{
	public:
		virtual const char* TypeName() = 0;
	};

	class ResourceBase
	{
	protected:
		ResGUID guid;
		FileSz size;
		TypedHandle<ResourceManager> manager;
		ExtraData* extra;
	public:
		const ResGUID& GUID() const { return guid; }
		const FileSz Size() const { return size; }
		ExtraData* Extra() { return extra; }
		const ExtraData* Extra() const { return extra; }
		void SetExtra(ExtraData* val) { extra = val; }
		ResourceBase(	const ResGUID& guidParam, FileSz sizeParam, 
					TypedHandle<ResourceManager> managerHnd, ExtraData* extraParam = NULL) :
				guid(guidParam)
		{
			size = sizeParam;
			manager = managerHnd;
			extra = extraParam;
		}
		virtual ~ResourceBase(void);
	};

	class Resource : public ResourceBase
	{
	protected:
		char* buffer;
	public:
		const char* Buffer() const { return buffer; }
		char* WriteableBuffer() { return buffer; }

		Resource(	const ResGUID& guidParam, char* bufferParam, FileSz sizeParam, 
					TypedHandle<ResourceManager> managerHnd, ExtraData* extraParam = NULL) :
				ResourceBase(guidParam, sizeParam, managerHnd, extraParam)
		{
			buffer = bufferParam;
		}
		~Resource(void);
	};

	/**
	Holds info and functions for a resource that can be decoded in chunks rather than in one burst,
	like audio. For this type, Size() returns the size of the raw buffer.
	This class must be passed custom read, seek, and closing functions. Any functions that are not supplied
	are replaced with nonfunctional stubs.
	*/
	class DecodableResource : public Resource
	{
	public:
		typedef FileSz (*ReadFunc)(DecodableResource* res, char* dest, FileSz numBytes);
		typedef FileSz (*SeekFunc)(DecodableResource* res, FileSz offset);
		typedef void (*CloseFunc)(DecodableResource* res);
	private:
		/**
		Current position of the read head in the resource.
		*/
		FileSz streamPos;
		ReadFunc readFunc;
		SeekFunc seekFunc;
		CloseFunc closeFunc;
		static FileSz defReadFunc(DecodableResource* res, char* dest, FileSz numBytes);
		static FileSz defSeekFunc(DecodableResource* res, FileSz offset);
		static void defCloseFunc(DecodableResource* res);
	public:
		DecodableResource(	const ResGUID& guidParam, char* bufferParam, FileSz sizeParam, 
							TypedHandle<ResourceManager> managerHnd, ExtraData* extraParam = NULL,
							ReadFunc pReadFunc = NULL, SeekFunc pSeekFunc = NULL, CloseFunc pCloseFunc = NULL);
		~DecodableResource(void);
		/**
		Attempts to copy the given number of bytes to the destination buffer.
		Returns the number of bytes actually decoded and copied; also moves the decoding head down by at most that many bytes. 
		If the requested decode size is more than that available, returns all remaining decodable bytes.
		*/
		FileSz Read(char* dest, FileSz numBytes);
		FileSz Seek(FileSz relOffset);
		void Close();
	};

	inline bool operator== (const ResourceBase& lhs, const ResourceBase& rhs) { return lhs.GUID() == rhs.GUID(); }
	inline bool operator!= (const ResourceBase& lhs, const ResourceBase& rhs) { return !(lhs == rhs); }
}