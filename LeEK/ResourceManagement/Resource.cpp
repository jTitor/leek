#include "Resource.h"
#include "Math/MathFunctions.h"
#include "Logging/Log.h"
#include "ResourceManagement/ResourceManager.h"
#include "FileManagement/Filesystem.h"
#include "Platforms/IPlatform.h"

using namespace LeEK;

void ResGUID::init(const String& name)
{
	memset(Name, 0, MAX_NAME_LEN);
	strncpy(Name, name.c_str(), MAX_NAME_LEN);
	Name[MAX_NAME_LEN - 1] = 0;
	//now run through the string.
	//catch the file separator
	char* tempArchSepPos = strchr(Name, ARCHIVE_SEPARATOR);
	archSepPos = 0;
	extPos = 0;
	if(!tempArchSepPos)
	{
		LogV(String("ResGUID ") + Name + " is missing a resource archive prefix!");
	}
	else
	{
		archSepPos = tempArchSepPos - Name;
	}

	//now process the rest of the GUID.
	for(U32 i = archSepPos; i < MAX_NAME_LEN; ++i)
	{
		char& curChar = Name[i];

		//unlikely, but you never know.
		if(curChar == '\\')
		{
			curChar = DIR_SEPARATOR;
		}
		else if(curChar == EXT_SEPARATOR)
		{
			//there should only be one extension!
			if(extPos)
			{
				LogV(String("ResGUID ") + Name + " has multiple extensions!");
			}
			extPos = i;
		}
		else if(curChar == 0)
		{
			break;
		}
	}
}

ResGUID::ResGUID(const char* name)
{
	String finalName(name);
	init(finalName);
}

ResGUID::ResGUID(const String& archName, const String& resName)
{
	String finalName = archName + ARCHIVE_SEPARATOR + ToLower(resName);
	init(finalName);
}

ResGUID::ResGUID()
{
	//init as an empty GUID
	memset(Name, 0, MAX_NAME_LEN);
	archSepPos = 0;
	extPos = 0;
}

U32 ResGUID::Length() const
{
	return strlen(Name);
}

String ResGUID::ResArchiveName() const
{
	if(archSepPos == 0)
	{
		return String("");
	}
	char tempBuf[MAX_ARCHIVE_NAME];
	strncpy(tempBuf, Name, archSepPos);
	tempBuf[archSepPos] = 0;
	return String(tempBuf);
}

String ResGUID::Filename() const
{
	const char* result = strrchr(Name, DIR_SEPARATOR) ;
	if(!result)
	{
		return "";
	}
	return result + 1;
}

String ResGUID::BaseName() const
{
	String fileName = Filename();
	if(fileName.c_str()[0] == 0)
	{
		return "";
	}
	char tempBuf[MAX_ARCHIVE_NAME];
	strncpy(tempBuf, fileName.c_str(), strlen(Name));
	//null out the last extension
	char* extPos = strrchr(tempBuf, EXT_SEPARATOR);
	if(extPos)
	{
		extPos[0] = 0;
	}
	return String(tempBuf);
}

bool ResGUID::IsArchiveGUID()
{
	//Basically check that there's an extension
	//to the archive prefix.
	return ResArchiveName().rfind(EXT_SEPARATOR) != String::npos;
}

bool ResGUID::IsRelFileSysGUID()
{
	if(!Filesystem::GetPlatform())
	{
		return false;
	}
	return !IsAbsFileSysGUID();
}

bool ResGUID::IsAbsFileSysGUID()
{
	auto plat = Filesystem::GetPlatform();
	if(!plat)
	{
		return false;
	}
	//Check for one of two things -
	//there's a colon in the archive prefix (Windows)
	//or the archive prefix starts with a "/" (everything else?).
	String archName = ResArchiveName();
	//TODO
	return false;
}

ResourceBase::~ResourceBase()
{
	//Get rid of the extra data if necessary.
	LogV(String("Deleting resource ") + guid.ResName());
	LDelete(extra);
}

Resource::~Resource(void)
{
	
	CustomArrayDelete(buffer);
	
	//inform the manager that a resource has been freed
	manager->ReportMemoryFreed(size);
}

FileSz DecodableResource::defReadFunc(DecodableResource* res, char* dest, FileSz numBytes)
{
	return 0;
}

FileSz DecodableResource::defSeekFunc(DecodableResource* res, FileSz offset)
{
	return 0;
}

void DecodableResource::defCloseFunc(DecodableResource* res)
{
}

DecodableResource::DecodableResource(	const ResGUID& guidParam, char* bufferParam, FileSz sizeParam, 
										TypedHandle<ResourceManager> managerHnd, ExtraData* extraParam,
										ReadFunc pReadFunc, SeekFunc pSeekFunc, CloseFunc pCloseFunc) :
										Resource(guidParam, bufferParam, sizeParam, managerHnd, extraParam)
{
	streamPos = 0;
	readFunc = pReadFunc ? pReadFunc : defReadFunc;
	seekFunc = pSeekFunc ? pSeekFunc : defSeekFunc;
	closeFunc = pCloseFunc ? pCloseFunc : defCloseFunc;
}

DecodableResource::~DecodableResource()
{
}

FileSz DecodableResource::Read(char* dest, FileSz numBytes)
{
	return readFunc(this, dest, numBytes);
}

FileSz DecodableResource::Seek(FileSz relOffset)
{
	streamPos = Math::Clamp(relOffset, (FileSz)0, Size());
	return seekFunc(this, streamPos);
}

void DecodableResource::Close()
{
	return closeFunc(this);
}