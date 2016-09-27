#include "Allocator.h"
#include "StdAfx.h"
#include "Memory/HeapLayers.h"
#include "Logging/Log.h"
#include "FileManagement/Filesystem.h"
#ifdef WIN32
//#include "Platforms/Win32Platform.h"
#include "Platforms/Win32Helpers.h"
#endif
#include "FileManagement/DataStream.h"
#include "Time/DateTime.h"
#include <cstdio>
using namespace LeEK;

//tracking system constants
//hash table needs to be prime sized for proper distribution.
const U32 ALLOC_TABLE_SIZE = 65521;
const U32 TAG_TABLE_SIZE = 128;

typedef StrictSegLayer<	7, //10 bins - a bit excessive?
		StrictSegLayerTraits, //strict segregated traits
		PagedFreeListLayer<DebugLogLayer<OSVirtualLayer>>, //use paged freelists for small allocations
		RBTreeTagLayer<DebugLogLayer<OSVirtualLayer>> //use a red-black tree-based tag system for large allocations
		> DebugHeap;

#ifndef DEBUG
#define DEBUG 1
#endif
#ifdef DEBUG
typedef DebugHeap Heap;
#else
typedef StrictSegLayer<	7, //10 bins - a bit excessive?
						StrictSegLayerTraits, //strict segregated traits
						PagedFreeListLayer<OSVirtualLayer>, //use paged freelists for small allocations
						RBTreeTagLayer<OSVirtualLayer> //use a red-black tree-based tag system for large allocations
						> Heap;
#endif

typedef PagedFreeListLayer<OSVirtualLayer> TrackerHeap;
Heap heap;
Heap bulletHeap;
//strings need their own damn heap, since they resize.
//the resize process makes a variety of different size allocations
//which causes a lot of pages to be requested from the freelist
//(probably something I did wrong, gotta check one of these days)
RBTreeTagLayer<OSVirtualLayer> strHeap;
//STL uses its own heap for now too.
Heap stlHeap;

//here's all the tracking subsystems
TrackerHeap tagHeap;
//these are fixed size hashtables so that we don't have to deal with a constantly growing map
TagDesc* tagTable[TAG_TABLE_SIZE];
//alternately, we could have a hashtable for the allocDescs themselves?
AllocDesc* allocTable[ALLOC_TABLE_SIZE];
TrackerHeap allocDescHeap;
Allocator allocatorInst;

//logging vars
bool verboseDump = false;
//TODO: make a way to allocate from a debug heap?
const U32 SUMMARY_MAX = 1024;
char allocSummary[SUMMARY_MAX];

//stats
//move these to AllocStats!
F64 peakAllocsKb = 0;
F64 totAllocsKb = 0;

extern void* operator new(size_t, void* ptr, custom_tag)
{
	return ptr;
}
extern void* operator new[](size_t, void* ptr, custom_tag)
{
	return ptr;
}
extern void operator delete(void* ptr, void*, custom_tag)
{
	Allocator::_CustomFree(ptr);
}
extern void operator delete[](void* ptr, void*, custom_tag)
{
	Allocator::_CustomFree(ptr);
}

Allocator::Allocator(void)
{
	//zero out the hashtables
	memset(tagTable, 0, sizeof(tagTable));
	memset(allocTable, 0, sizeof(allocTable));
	//also setup default tracking systems
	//don't check the bullet allocator, since we don't know if we can defrag at will

}


Allocator::~Allocator(void)
{
}

TagDesc* TagDesc::Register(U32 category, const char* tagName, size_t size)
{
	L_ASSERT(strcmp(tagName, "") != 0);
	//U32 tagCRC = Hash(tagName);
	//hash DOES need to fit within the hash table.
	U32 tagHash = category;//tagCRC & (TAG_TABLE_SIZE - 1);
	TagDesc* currTag = tagTable[tagHash];//category];
	if(currTag)
	{
		//TagDesc* currTag = tagInMap->second;
		//if the tag exists...
		if(currTag->CRC == TagDesc::TAG_GUARD)//tagCRC)
		{
			//update the alloc size total
			currTag->Size += size;
			//and return the tag
			return currTag;
		}
		//otherwise, go to the next tag being pointed to
		//LogV(String("Couldn't find tag at ") + StrFromVal(tagHash) + String(", going to ") + StrFromVal((void*)currTag->Next));
		//currTag = currTag->Next;
	}
	//otherwise, we need to register a new tag
	currTag = (TagDesc*)tagHeap.Malloc(sizeof(TagDesc));
	currTag->Category = category;
	currTag->CRC = TagDesc::TAG_GUARD;//tagCRC;
	currTag->Size = size;
	//do the name copy here
	//we'll do by straight memcpy
	U32 tagLen = Math::Min(strlen(tagName), (size_t)(TagDesc::MAX_STR_LEN - 1));
	memcpy(currTag->TagName, tagName, tagLen);
	//remember to put the null at the end!
	currTag->TagName[tagLen] = 0;
	//the next pointer should point to nothing, this is the newest tag
	//currTag->Next = tagTable[tagHash];
	tagTable[tagHash] = currTag;

	return currTag;
}

U32 getPtrHash(void* ptr)
{
	return ((size_t)ptr) % (ALLOC_TABLE_SIZE);
}

void registerAlloc(void* ptr, size_t size, U32 category, const char* desc, const char* file, U32 line)
{
	AllocDesc* alloc = (AllocDesc*)allocDescHeap.Malloc(sizeof(AllocDesc));
	alloc->Ptr = ptr;
	alloc->Size = size;
	alloc->Line = line;
	//do NOT do the name copy.
	//not sure how this would work, but hey!
	alloc->File = file;
	//see if we don't already have the tag in the tag system
	alloc->Tag = TagDesc::Register(category, desc, size);

	//now put the alloc in a hash table too
	U32 ptrHash = getPtrHash(ptr);
	//alloc->Next = allocTable[ptrHash];
	if(allocTable[ptrHash] != 0)
	{
		//Log::RAW("Alloc table collision!\n");
	}
	allocTable[ptrHash] = alloc;
}

void updateAlloc(void* ptr, void* target, size_t newSize, const char* file, U32 line)
{
	//find the alloc
	//first get the hash again
	U32 ptrHash = getPtrHash(ptr);
	AllocDesc* alloc = allocTable[ptrHash];
	//now we need to change data as necessary
	U32 newHash = getPtrHash(target);
	AllocDesc* newAlloc = (AllocDesc*)allocDescHeap.Malloc(sizeof(AllocDesc));
	//now fill in altered data
	memcpy(newAlloc, alloc, sizeof(AllocDesc));
	newAlloc->Ptr = target;
	newAlloc->Size = newSize;
	newAlloc->Line = line;
	newAlloc->File = file;
	//newAlloc->Next = allocTable[newHash];
	allocTable[newHash] = newAlloc;
	//now update the tag
	//remove the old alloc, add the new alloc
	alloc->Tag->Size -= alloc->Size;
	newAlloc->Tag->Size += newAlloc->Size;
	//if the two hashes don't match, remove the old alloc
	if(ptrHash != newHash)
	{
		allocTable[ptrHash] = NULL;
	}
	//and free the old alloc data
	allocDescHeap.Free(alloc);
}

const AllocDesc* getAlloc(void* ptr)
{
	U32 ptrHash = getPtrHash(ptr);
	return allocTable[ptrHash];
}

void unregisterAlloc(void* ptr)
{
	//find the alloc
	//first get the hash again
	U32 ptrHash = getPtrHash(ptr);
	AllocDesc* alloc = allocTable[ptrHash];
	if(alloc)
	{
		//now we can decrement from the tag
		alloc->Tag->Size -= alloc->Size;
		//now remove this alloc
		allocTable[ptrHash] = NULL;
		//and free the memory
		allocDescHeap.Free(alloc);
	}
}

void* Allocator::_CustomMalloc(size_t size, U32 allocType, const char* desc, const char* file, U32 line)
{
	void* result = heap.Malloc(size);
	//do any needed bookkeeping here
	registerAlloc(result, size, allocType, desc, file, line);
	L_ASSERT(result && "Couldn't make alloc!");
	return result;
}

void* Allocator::_CustomRealloc(void* target, size_t newSize, const char* file, U32 line)
{
	void* result = heap.Realloc(target, newSize);
	//do any needed bookkeeping here
	updateAlloc(target, result, newSize, file, line);
	return result;
}

void Allocator::_CustomFree(void* target)
{
	unregisterAlloc(target);
	heap.Free(target);
	target = NULL;
}

void* Allocator::_AlignedMalloc(size_t size, size_t alignment, U32 allocType, const char* desc, const char* file, U32 line)
{
	const size_t SLACK = alignment + sizeof(void*);
	//Simple idea - allocate a little more memory than needed,
	//enough to ensure a section in the alloc's aligned.
	//Also include a tiny bit of space to store a pointer to the raw alloc
	size_t allocSize = size + SLACK;
	void* rawMem = _CustomMalloc(allocSize, allocType, desc, file, line);
	//Shift up pointer until it's aligned, return that pointer.
	void **res = (void**)(((size_t)rawMem+SLACK) & ~(alignment - 1));
	//Store the original pointer right behind the result alloc
	res[-1] = rawMem;
	return res;
}

void* Allocator::_AlignedRealloc(void* target, size_t newSize, size_t alignment, const char* file, U32 line)
{
	//Get the old alloc's info.
	auto allocData = getAlloc(target);
	U32 allocType = allocData->Tag->Category;
	char desc[TagDesc::MAX_STR_LEN] = {0};
	strcpy_s(desc, allocData->Tag->TagName);
	//Free the old data...
	_AlignedFree(target);
	//Now give a new aligned pointer.
	return _AlignedMalloc(newSize, alignment, allocType, desc, file, line);
}

void Allocator::_AlignedFree(void* target)
{
	//Trivial thanks to the bookkeeping pointer -
	//Move back by a pointer, and free that address
	_CustomFree(((void**)target)[-1]);
}

//returns all unused memory to the OS.
void Allocator::Purge()
{
	//The heap layers also need to inform the handle system of purged regions.
	heap.Purge();
	bulletHeap.Purge();
	strHeap.Purge();
	tagHeap.Purge();
	stlHeap.Purge();
}

#pragma region External Hooks
void* Allocator::BulletMalloc(size_t size)
{
	void* result = bulletHeap.Malloc(size);
	//do any needed bookkeeping here
	registerAlloc(result, size, BULLET_ALLOC, "BulletAlloc", "BulletLibrary", 0);
	L_ASSERT(result && "Couldn't make Bullet alloc!");
	return result;
}

void Allocator::BulletFree(void* target)
{
	unregisterAlloc(target);
	bulletHeap.Free(target);
}

void* Allocator::STLStrMalloc(size_t size)
{
	void* result = strHeap.Malloc(size);
	//do any needed bookkeeping here
	registerAlloc(result, size, STRING_ALLOC, "STLStrAlloc", "STL", 0);
	L_ASSERT(result && "Couldn't make STL string alloc!");
	return result;
}

void Allocator::STLStrFree(void* target)
{
	unregisterAlloc(target);
	strHeap.Free(target);
}

void* Allocator::STLMalloc(size_t size)
{
	void* result = stlHeap.Malloc(size);
	//do any needed bookkeeping here
	registerAlloc(result, size, AllocType::STLHOOK_ALLOC, "STLHookAlloc", "STL", 0);
	L_ASSERT(result && "Couldn't make STL string alloc!");
	return result;
}

void Allocator::STLFree(void* target, size_t freedSize)
{
	unregisterAlloc(target);
	stlHeap.Free(target);
}
#pragma endregion

void Allocator::SetVerboseDump(bool val)
{
	verboseDump = val;
}

//TODO: move to AllocStats
void updateAllocsTotal(F64 newTotKb)
{
	totAllocsKb = newTotKb;
	peakAllocsKb = Math::Max(totAllocsKb, peakAllocsKb);
}

void Allocator::WriteAllocsCSV(DataStream* file)
{
	//write the tag totals first
	char lineBuffer[256];
	file->WriteLine("Category,Category Size (kB)");
	F64 totMemUsed = 0;
	for(U32 i = 0; i < TAG_TABLE_SIZE; i++)
	{
		TagDesc* tag = tagTable[i];
		if(tag && tag->CRC == TagDesc::TAG_GUARD)
		{
			totMemUsed += tag->Size;
			sprintf_s(lineBuffer,sizeof(lineBuffer), "%s,%.3f", tag->TagName, (F64)(tag->Size) / 1024);
			file->WriteLine(lineBuffer);
			//clear the buffer
			lineBuffer[0] = 0;
		}
	}
	updateAllocsTotal(totMemUsed / 1024);
	file->WriteLine();
	file->WriteLine("Total Dynamic Allocations (kB),Peak Allocs (kB)");
	sprintf_s(lineBuffer,sizeof(lineBuffer), "%.3f,%.3f", totAllocsKb, peakAllocsKb);
	file->WriteLine(lineBuffer);
	//write the individual allocs if necessary
	if(verboseDump)
	{
		file->WriteLine();
		file->WriteLine("Address,Category,Category Size,Alloc Size,File,Line");
		//now write all the allocs
		for(U32 i = 0; i < ALLOC_TABLE_SIZE; i++)
		{
			AllocDesc* alloc = allocTable[i];
			if(alloc)
			{
	#ifdef WIN32
				sprintf_s(	lineBuffer, sizeof(lineBuffer), "0x%08x,%s,%d,%d,%s,%d",
							alloc->Ptr, alloc->Tag->TagName, alloc->Tag->Size, 
							alloc->Size, alloc->File, alloc->Line);
	#endif
				file->WriteLine(lineBuffer);
				lineBuffer[0] = 0;
				//alloc = alloc->Next;
			}
		}
		//could also write the tags to separate columns, but can consider that later
	}
}

void Allocator::DumpAllocsCSV(const char* path)
{
	LogD("Dumping allocs...");
	//open up a file, of course
	DataStream* logFile = Filesystem::OpenFile(path);//Path((Filesystem::GetProgDir() + String("/AllocLog - ") + DateTime::GetCurrDate() + ".csv").c_str()));
	if(!logFile)
	{
		LogW("Couldn't open alloc log file!");
		return;
	}
	//try to append to any existing log
	logFile->SeekToEnd();
	if(logFile->FileSize() > 0)
	{
		logFile->WriteLine();
	}
	//and write the dump time
	//logFile->WriteLine(String("Dumped at time ") + DateTime::GetCurrLocalTime());

	WriteAllocsCSV(logFile);
	//we're done, remember to close the file
	logFile->Close();
	LogD("Dump complete.");
}

//displays how much each allocation category
//currently has allocated to the terminal.
const char* Allocator::FindAllocSummary()
{
	allocSummary[0] = 0;
	char lineBuffer[256];
	//logFile->WriteLine("Category,Category Size (kB)");
	strcat_s(allocSummary, SUMMARY_MAX, "Allocation Summary (kB):\n");
	F64 totMemUsed = 0;
	for(U32 i = 0; i < TAG_TABLE_SIZE; i++)
	{
		TagDesc* tag = tagTable[i];
		if(tag)
		{
			if(tag->CRC == TagDesc::TAG_GUARD)
			{
				totMemUsed += tag->Size;
				sprintf_s(lineBuffer, sizeof(lineBuffer), "%s:\t%.3f\n", tag->TagName, (F64)(tag->Size) / 1024);
				strcat_s(allocSummary, SUMMARY_MAX, lineBuffer);
				//terminate the string
				lineBuffer[0] = 0;
			}
		}
	}
	updateAllocsTotal(totMemUsed / 1024);
	strcat_s(allocSummary, SUMMARY_MAX, "\n");
	strcat_s(allocSummary, SUMMARY_MAX, "Total Dynamic Allocations (kB): ");
	sprintf_s(lineBuffer, sizeof( lineBuffer), "%.3f\n", totAllocsKb);
	strcat_s(allocSummary, SUMMARY_MAX, lineBuffer);
	strcat_s(allocSummary, SUMMARY_MAX, "Peak Allocs (kB): ");
	sprintf_s(lineBuffer, sizeof(lineBuffer), "%.3f\n", peakAllocsKb);
	strcat_s(allocSummary, SUMMARY_MAX, lineBuffer);
	return allocSummary;//.c_str();
}

F64 Allocator::TotalMemoryAllocated()
{
	return totAllocsKb;
}

F64 Allocator::PeakMemoryAllocated()
{
	return peakAllocsKb;
}