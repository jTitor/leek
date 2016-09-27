#pragma once
#include "StdAfx.h"
#include "Datatypes.h"
#include "Platforms/IPlatform.h"
#include "Math/MathFunctions.h"
#include "DataStructures/RedBlackTreeBase.h"
#include "DataStructures/IntrusiveList.h"
#include "Logging/Log.h"
#include "Constants/LogTags.h"
#include "Stats/AllocStats.h"
#include <cstdlib>
//platform dependent headers
#ifdef WIN32
#include "Platforms/Win32Helpers.h"
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

#ifndef NULL
#define NULL 0
#endif

namespace LeEK
{
	//HeapLayers.h
	//Defines the classes that form the memory allocator via the Heap Layer pattern.
	//might want to make a Purge() method?
	//inline void* OSVirAlloc(void* desiredAddr, size_t size);
	//inline void OSVirFree(void* ptr);

	template<class T> inline T roundDown(T x, size_t a) {return x & -(int)a;}
	template<class T> inline T roundUp(T x, size_t a) {return (x + (a-1)) & -(int)a;}
	//rounds passed pointer to START of given boundary (the boundary's leading address)
	static const U32 PAGE_SIZE = 65536;

	template<class T> inline T* AlignDown(T* p, size_t a)
	{
		//the - operator in this case acts as logical negation
		//thus & will mask out the top bit of the boundary
		return (T*)((size_t)p & -(int)a);
	}
	//rounds passed pointer to END of given boundary
	template<class T> inline T* AlignUp(T* p, size_t a) {return (T*)(((size_t)p + (a-1)) & -(int)a);}

	template<class SuperLayer> class DebugLogLayer : public SuperLayer
	{
	private:
		const static U8 MAX_INT_DIGITS = 32;
		const static U32 BUF_SIZE = 4*MAX_INT_DIGITS;
		char buffer[BUF_SIZE];
		public:
		inline void* Malloc(size_t size)
		{
			sprintf_s(buffer, sizeof(buffer), "Allocating %d bytes", size);
			std::cout << buffer << "\n";
			return SuperLayer::Malloc(size);
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			sprintf_s(buffer, sizeof(buffer), "Reallocating object to %d bytes", size);
			std::cout << buffer << "\n";
			return SuperLayer::Realloc(ptr, size);
		}

		inline void Free(void* ptr, size_t size = 0)
		{
			//size_t targetSize = GetSize(ptr);
			sprintf_s(buffer, sizeof(buffer), "Freeing pointer %p", ptr);
			std::cout << buffer << "\n";
			SuperLayer::Free(ptr);
		}
	};

	/**
	 * Default runtime allocating layer,
	 * makes allocations via malloc() and free().
	 * Unfortunately, there's no easy way to know how much is being freed on a free;
	 * then again, this shouldn't be used outside of test code.
	 */
	class CRTLayer
	{
	public:
		inline void* Malloc(size_t size)
		{
			void* res = malloc(size);
			return res;
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			return realloc(ptr, size);
		}

		inline void Free(void* ptr)
		{
			if(ptr)
			{
				free(ptr);
			}
		}

		inline void Purge()
		{
			//do nothing, system handles memory
		}
	};

	/**
	 * Virtual memory allocator. Use to allocate large chunks of memory
	 * that other layers split up into individual allocations.
	 * The allocation should be aligned to the size's nearest power of 2.
	 */
	class OSVirtualLayer
	{
		private:
		//platform-specific allocation functions
		//generally provide aligned allocations
		inline void* osVirAlloc(void* desiredAddr, size_t size)
		{
			void* res = NULL;
		#ifdef WIN32
			//the page will be aligned as needed
			//under Win32.
			res = VirtualAlloc(desiredAddr,		//allocate at or near hinted address
								size,				//allocate this large a block
								MEM_COMMIT,			//whatever page we get, mark as ready for reading and writing
								PAGE_READWRITE);	//we may read or write data in the page
		#else
			//Under POSIX, it's a little different...
			//We need to ensure the allocation's aligned,
			//mmap won't do that for us.
			size_t align = size;//Math::NearestPowOf2(size);
			void* rawMem = mmap(	desiredAddr, size + align,
						PROT_READ | PROT_WRITE,
						MAP_PRIVATE | MAP_ANONYMOUS,
						-1, 0);
			if(rawMem == MAP_FAILED || rawMem == NULL)
			{
				Log::RAW("Failed to allocate virtual memory!\n");
				return NULL;
			}
			//Now we should have a compatible block.
			//Align the alloc, so we know what gaps to immediately unmap.
			res = (void*)((((size_t)rawMem) + align) & ~(align - 1));
			//Unmap the region from mem start to the result,
			size_t backGapSize = (((size_t)res) - ((size_t)rawMem));
			munmap(rawMem, backGapSize);
			//and unmap the region from end of the result block to (remaining slack space).
			size_t frontGapSize = align - backGapSize;
			munmap((void*)(((char*)res) + size), frontGapSize);
		#endif
			//only record the alloc if the alloc succeeded
			if(res)
			{
				ReportOSAlloc(size);
			}
			return res;
		}

		inline void osVirFree(void* ptr, size_t size)
		{
			size_t freedSize = size;
		#ifdef WIN32
			//similar to VirtualAlloc call
			//will attempt to free, but may fail if given the wrong pointer
			//(must be same pointer as given in OSVirAlloc() call)
			//will print an error on failure

			//get info on the page
			MEMORY_BASIC_INFORMATION memInf;
			size_t result = VirtualQuery(ptr, &memInf, sizeof(memInf));
			//a query on the page info will only be valid if the whole struct is written to
			L_ASSERT(result == sizeof(memInf) && "Couldn't get virtual page info!");
			/*
				Try to free the entire allocated region.
				Attempting partial frees didn't work,
				it gives an error that "the parameter was incorrect".

				Because the entire region's being freed,
				you have to ensure all layers don't have any housekeeping
				structures in the region on a OS free.
			*/
			if(!VirtualFree(ptr, 0, MEM_RELEASE))
			{
				LogW("Attempt to free a page failed!");
				Win32Helpers::LogLastError();
				return;
			}
			freedSize = memInf.RegionSize;
		#else
			int err = munmap(ptr, size);
			if(err != 0)
			{
				Log::RAW("Failed to free virtual page!\n");
				return;
			}
		#endif
			//report the page free
			//memInf.RegionSize should be the size of the page...
			RemoveOSAlloc(freedSize);
			//DEFINITELY notify the handle system of this.
			HandleMgr::NotifyRegionPurged(ptr, freedSize);
		}

		public:
		inline void* Malloc(size_t size)
		{
			return osVirAlloc(NULL, size);
		}

		//WTF?
		//why would we realloc virtual-allocated memory?
		//these should ordinarily be HUGE blocks in the first place
		//and can't easily be moved
		inline void* Realloc(void* ptr, size_t size)
		{
			LogW("Attempting to reallocate virtual-addressed memory!");
			return ptr;//realloc(ptr, size);
		}

		inline void Free(void* ptr, size_t size = 0)
		{
			if(ptr)
			{
				osVirFree(ptr, size);
			}
		}
		//OSVirAlloc(NULL, PAGE_SIZE);
	};

	template<class SuperLayer> class SentinelLayer : public SuperLayer
	{
	private:
		const static U32 SENTINEL_BUF_LEN = 8;
		const static U8 SENTINEL_VAL = 0xA5;
	public:
		inline void* Malloc(size_t size)
		{
			//we're going to place sentinels at the start of the data
			char* res = (char*)SuperLayer::Malloc(size + SENTINEL_BUF_LEN);
			memset(res, SENTINEL_VAL, SENTINEL_BUF_LEN);
			//and return the data pointer specifically
			return (void*)(res + SENTINEL_BUF_LEN);
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			return SuperLayer::Realloc((void*)(((char*)ptr) - SENTINEL_BUF_LEN), size + SENTINEL_BUF_LEN);
		}

		inline void Free(void* ptr)
		{
			if(ptr)
			{
				//check the sentinel area.
				//if it differs from the sentinel value at all,
				//memory was corrupted.
				for(U32 i = 0; i < SENTINEL_BUF_LEN; ++i)
				{
					const unsigned char& val = ((char*)ptr)[-SENTINEL_BUF_LEN + i];
					L_ASSERT(val == SENTINEL_VAL && "Memory was corrupted!");
				}
				SuperLayer::Free((void*)(((char*)ptr) - SENTINEL_BUF_LEN));
			}
		}
	};

	//Free list layer
	//Allocates large (64k) pages, divided into a specified size
	template<class SuperLayer> class PagedFreeListLayer : SuperLayer
	{
		//General procedure is:
		//If a page has enough free space for an allocation,
		//return the next available space.
		//To free a pointer, put the pointer in the free list.
		//Allocs must be >= sizeof(void*),
		//since the pointer to another free node is stored in each free node.

		struct PageHeader : public IntrusiveListNode
		{
		private:
			//unfortunately messes w/ byte alignment.
			size_t blockSize;
			U8* firstFreeNode;
		public:
			U32 HeaderGuard;
		private:
			U16 usedBlocks;
		public:
			const static U32 HEADER_GUARD = 0xDECADEAF;

			PageHeader(size_t blSz) : blockSize(blSz), HeaderGuard(HEADER_GUARD)
			{
				usedBlocks = 0;
				firstFreeNode = NULL;
			}
			inline const size_t BlockSize() { return blockSize; }
			inline const U16 NumBlocks() { return (PAGE_SIZE - sizeof(PageHeader)) / blockSize; }
			const U16 UsedBlocks() { return usedBlocks; }
			//const U32 HeaderGuard() { return HeaderGuard; }
			inline bool Empty() { return usedBlocks == 0; }
			inline bool Full() { return firstFreeNode == NULL; }
			inline void markNodeFree(void* ptr)
			{
				//place the link to the next free node in this pointer
				*(U8**)ptr = firstFreeNode;
				//and make the target the head of the free list
				firstFreeNode = (U8*)ptr;
			}
			inline U8* FirstFreeNode() { return firstFreeNode; }
			void ReportBlockAlloc()
			{
				L_ASSERT(!Full() && "Tried to alloc blocks in a full page!");
				usedBlocks++;
				//also set the next free node
				firstFreeNode = *((U8**)firstFreeNode);
			}
			void ReportBlockFree()
			{ 
				L_ASSERT(!Empty() && "Tried to free blocks in an empty page!");
				usedBlocks--;
			}
			void operator=(const PageHeader& rhs)
			{
				usedBlocks = rhs.usedBlocks;
				blockSize = rhs.blockSize;
				firstFreeNode = rhs.firstFreeNode;
				HeaderGuard = rhs.HeaderGuard;
				L_ASSERT(HeaderGuard == HEADER_GUARD && "Tried to copy corrupt page header!");
			}
		};

		//head of the list should be the most recently free page
		IntrusiveList pageList;

		static PageHeader* getPageHeader(void* pageStart)
		{
			char* headerPos = ((char*)pageStart) + PAGE_SIZE - sizeof(PageHeader);
			PageHeader* res = (PageHeader*)((void*)headerPos);
			return res;
		}

		static void* getPageStart(PageHeader* header)
		{
			char* headerPos = (char*)((void*)header);
			return headerPos + sizeof(PageHeader) - PAGE_SIZE;
		}

	public:
		PagedFreeListLayer() : pageList()//, blockSize(0)
		{}
		//make destructor that frees all pages
		void* Malloc(size_t size)
		{
			if(!pageList.Root()->Prev)
			{
				pageList.Init();
			}
			//if there's no free pages, we need to allocate a new page
			if(pageList.Empty() || ((PageHeader*)pageList.Front())->Full())
			{
				//each entry needs to be able to store a node of the free list
				size = Math::Max(size, sizeof(void*));
				const U32 allocsPerPage = (PAGE_SIZE - sizeof(PageHeader)) / size;
				//allocate the page here
				char* newPage = (char*)SuperLayer::Malloc(PAGE_SIZE);//OSVirAlloc(NULL, PAGE_SIZE);
				L_ASSERT(newPage == AlignDown(newPage, PAGE_SIZE));
				//zero out this memory
				memset(newPage, 0, PAGE_SIZE);
				//put the size data at the end of the page
				PageHeader* headerPtr = getPageHeader(newPage);
				headerPtr[0] = PageHeader(size);
				L_ASSERT(headerPtr->HeaderGuard == PageHeader::HEADER_GUARD && "Failed to create valid page!");
				//also record the overhead
				ReportOverhead(sizeof(PageHeader));
				//and start the actual memory
				U8* newFreeNode = (U8*)&newPage[0];
				//build the page's free list in one fell swoop
				for(U32 i = 0; i < allocsPerPage; i++)
				{
					//place this block of the new page in the free list
					//and move to the next block
					headerPtr->markNodeFree(newFreeNode);
					newFreeNode += size;
				}
				//and add it to the head of the free list
				pageList.AddToFront(headerPtr);
			}
			//there should definitely be a free page now
			PageHeader* header = (PageHeader*)pageList.Front();
			L_ASSERT(header->HeaderGuard == PageHeader::HEADER_GUARD && "Attempting Malloc() on corrupt page!");
			//Take the page's current free node
			U8* result = header->FirstFreeNode();
			//Also notify the header that that node's been used
			header->ReportBlockAlloc();
			//if this would fill the page, move the page to the back
			if(header->Full())
			{
				pageList.Remove(header);
				pageList.AddToBack(header);
			}

			return result;
		}

		inline void Free(void* ptr)
		{
			if(ptr)
			{
				//PageHeader* header = ((PageHeader*)AlignDown(ptr, PAGE_SIZE));
				void* pageStart = AlignDown(ptr, PAGE_SIZE);
				PageHeader* header = getPageHeader(pageStart);
				if(header->HeaderGuard != PageHeader::HEADER_GUARD)
				{
					return;
				}
				header->ReportBlockFree();
				header->markNodeFree(ptr);
				//move this page to the front of the list
				if((size_t)header != (size_t)pageList.Front())
				{
					pageList.Remove(header);
					pageList.AddToFront(header);
				}
			}
		}

		static inline size_t GetSize(void* ptr)
		{
			//align the pointer to the start of a page
			//and then pull the data at the start
			//(should be the page's size data if the pointer's been allocated by this layer)
			void* pageStart = AlignDown(ptr, PAGE_SIZE);
			PageHeader* header = getPageHeader(pageStart);
			if(header->HeaderGuard != PageHeader::HEADER_GUARD)
			{
				//Log::RAW("GetSize() attempt on corrupt page!\n");
				return 0;
			}
			return header->BlockSize();
		}

		inline void Purge()
		{
			//run through the page list
			PageHeader* header = (PageHeader*)pageList.Front();
			while(header != pageList.Root())
			{
				L_ASSERT(header->HeaderGuard == PageHeader::HEADER_GUARD && "Attempting Purge() on corrupt page!");
				PageHeader* nextHeader = (PageHeader*)header->Next;
				//any empty pages can be removed, and then returned to OS.
				if(header->Empty())
				{
					pageList.Remove(header);
					void* pageStart = getPageStart(header);
					SuperLayer::Free(pageStart, PAGE_SIZE);
					//now we need to inform the handle system that the memory is purged
					HandleMgr::NotifyRegionPurged(pageStart, PAGE_SIZE);
				}
				header = nextHeader;
			}
		}
	};

	//Size tagging layer
	//tacks on a header to each allocation indicating the size of the allocation
	template<class SuperLayer> class SizeTagLayer : SuperLayer
	{
		
		struct SizeTag
		{
			size_t Size;
			U32 IntegMask;
		};
		static const U32 maskVal = 0xDEADBEEF;

	public:
		inline void* Malloc(size_t size)
		{
			//we'll put the header at the start of the allocation
			SizeTag* ptr = (SizeTag*)SuperLayer::Malloc(size + sizeof(SizeTag));
			if(ptr)
			{
				ptr[0].Size = size;
				ptr[0].IntegMask = maskVal;
			}
			//report the header
			ReportOverhead(sizeof(SizeTag));
			//return just the data
			return (void*)&ptr[1];
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			SizeTag* origPtr = (SizeTag*)ptr;
			SizeTag* newPtr = (SizeTag*)SuperLayer::Realloc(	ptr ? &origPtr[-1] : NULL,
																	size + sizeof(SizeTag));
			if(newPtr)
			{
				newPtr[0].Size = size;
				newPtr[0].IntegMask = maskVal;
			}
			return (void*)&newPtr[1];
		}

		inline void Free(void* ptr)
		{
			//pass the full allocation to the next layer
			//other heap layers may be interested in the size header
			RemoveOverhead(sizeof(SizeTag));
			SizeTag* origPtr = (SizeTag*)ptr;
			SuperLayer::Free((void*)&origPtr[-1]);
		}

		static inline size_t GetSize(void* ptr)
		{
			SizeTag* origPtr = (SizeTag*)ptr;
			//size_t size = origPtr[-1];
			//pull the size tag only if the tag's mask is untampered
			if(origPtr[-1].IntegMask == maskVal)
			{
				return origPtr[-1].Size;//size;
			}
			else
			{
				return NULL;
			}
			//return ((FreeObject*)(&ptr[-1]))->Size;
		}
	};

	//sorting function struct (or a "Trait")
	//decides how to sort an allocation between various bins
	//this trait sorts by the allocation's size;
	//each bin stores allocations up to the previous bin's size * 2
	struct StrictSegLayerTraits
	{
		static inline U32 GetSizeClass(size_t size)
		{
			U32 binNum = 0;
			//decrement so it'll definitely be less than one of the bins' storage size
			size--;
			//find the next bin that'll fit the allocation
			//if the current bin doesn't, move to the next by bit-shifting the size down
			//(effectively divides the size by 2)
			while(size > 7)
			{
				size >>= 1;
				binNum++;
			}

			return binNum;
		}

		//returns the size of an allocation in a specified bin number
		//bin 0 will store a maximum allocation of 8 bytes
		static inline size_t GetClassMaxSize(U32 binNum)
		{
			return 1 << (binNum + 3);
		}
	};

	//strict segregated layer
	//allocation blocks aren't split or coalesced when no longer needed
	//cf. dlmalloc
	template<U32 NumBins, class Traits, class SmallLayer, class BigLayer> class StrictSegLayer : public BigLayer
	{
		//small allocations are routed to an array of layers,
		//split into max sizes that are powers of two
		//these are our bins
		SmallLayer smallLayers[NumBins];

		//as in StrictSegLayerTraits,
		//returns the bin an allocation falls into
		//if an allocation would be outside the bins, however,
		//then return (NumBins) itself to indicate a large allocation
		inline U32 InnerGetSizeClass(size_t size)
		{
			if(size > Traits::GetClassMaxSize(NumBins - 1))
			{
				return NumBins;
			}
			return Traits::GetSizeClass(size);
		}

	public:
		StrictSegLayer() {}

		inline void* Malloc(size_t size)
		{
			//note any wasted space
			//this might be a bit of a pain -
			//we know the space in Malloc, but not in Free.
			//to solve this, we can include a size tag for small allocs
			size_t dbgSize = size + sizeof(U16);
			//first, find the bin the alloc would fit in
			U32 sizeClass = 0;
			//TODO: predicate defs on ENABLE_ALLOC_STATS
#ifdef ENABLE_ALLOC_STATS
			sizeClass = InnerGetSizeClass(dbgSize);
#else
			sizeClass = InnerGetSizeClass(size);
#endif

			//if it's too large for bins,
			//use the big layer's allocator
			if(sizeClass >= NumBins)
			{
				return BigLayer::Malloc(size);
			}

			//otherwise, get the alloc size for the bin number
			U32 binAllocSize = Traits::GetClassMaxSize(sizeClass);
#ifdef ENABLE_ALLOC_STATS
			U16 waste = (U16)(binAllocSize - dbgSize);
			ReportWaste(waste);
			//also report the tag...
			ReportOverhead(sizeof(U16));
			//and make an alloc of that size
			U16* res = (U16*)smallLayers[sizeClass].Malloc(binAllocSize);
			//stick the waste into the tag
			res[0] = waste;
			//and return the data section
			return (void*)&res[1];
#else
			return smallLayers[sizeClass].Malloc(binAllocSize);
#endif
		}

		inline void* Realloc(void* ptr, size_t newSize)
		{
			//will be a little different
			//the superlayer for this layer is whatever's passed for BigLayer,
			//so BigLayer needs to support GetSize!!!
			if(ptr)
			{
				//free the pointer if we're reallocing to a 0-size allocation
				if(newSize)
				{
					//otherwise, see if this alloc would be in one of the bins
					size_t allocSize = GetSize(ptr);
					U32 allocSizeClass = InnerGetSizeClass(allocSize);

					//if not, go straight to the big layer
					if(allocSizeClass >= NumBins)
					{
						return BigLayer::Realloc(ptr, newSize);
					}

					//else, we'll need to do some work
					//we'll only realloc if the new size is larger than the old,
					//OR if it's at least half as small as the old size
					//(since those are the only scenarios where we'd need to move the allocation)

					if((newSize > allocSize) || (newSize < allocSize / 2))
					{
						 void* newPtr = Malloc(newSize);

						 if(newPtr)
						 {
							 size_t copySize = Math::Min(allocSize, newSize);
							 memcpy(newPtr, ptr, copySize);
						 }

						 //remove the old allocation
						 smallLayers[allocSizeClass].Free(ptr);

						 return newPtr;
					}
				}

				Free(ptr);
				return NULL;
			}

			return Malloc(newSize);
		}

		inline void Free(void* ptr)
		{
			if(ptr)
			{
				//as in realloc, we need to know if the alloc would be in a bin or the big layer
				//however, we can't just find a size_t tag, 
				//since if it was allocated by the small layer, the alloc wouldn't have a valid size_t
				//we need to have a validating mask
				size_t allocSize = GetSize(ptr);
				U32 allocSizeClass = InnerGetSizeClass(allocSize);
				if(allocSizeClass >= NumBins && allocSize != 0)
				{
					BigLayer::Free(ptr);
				}
				else
				{
					//if we know it's in the small alloc, we can go to the alloc's front
					//and get the waste
#ifdef ENABLE_ALLOC_STATS
					//report the waste and overhead
					U16* castPtr = (U16*)ptr;
					U16 waste = castPtr[-1];
					RemoveWaste(waste);
					RemoveOverhead(sizeof(U16));
					//and free the alloc
					smallLayers[allocSizeClass].Free((void*)&castPtr[-1]);
#else
					smallLayers[allocSizeClass].Free(ptr);
#endif
				}
			}
		}

		inline size_t GetSize(void* ptr)
		{
			//first, try to find the size via big heap
			//size_t allocSize = BigLayer::GetSize(ptr);
			size_t allocSize = SmallLayer::GetSize(ptr);
			//if that's invalid, then we now have to align the passed address to a page
			if(!allocSize)
			{
				allocSize = BigLayer::GetSize(ptr);
				//allocSize = SmallLayer::GetSize(ptr);
			}
			return allocSize;
		}

		//Frees empty pages in the allocator layers.
		inline void Purge()
		{
			//first purge the small layers
			for(U32 i = 0; i < NumBins; ++i)
			{
				smallLayers[i].Purge();
			}

			//then the big layer
			BigLayer::Purge();
		}
	};

	/*	next up is a RBTree'd layer (cf. GPG7, p.20)
		alloc process is:
			find node in tree w/ key >= desired size
			if key is too large, split block and return extra to tree
			if there's no blocks, get more in multiple pages
				(presumably the new page is one node?)
		free process is:
			get block's header (it's a node of the tree)
			see if neighbors are free
				if any are, coalesce them
				add resulting free block to tree
	*/
	//several features are still missing:
	//	!* page purging if page is no longer in use
	//	* checking against leaks on destruction (then again, none of the other layers do that, either)
	//for now, let's test it out
	template<class SuperLayer> class RBTreeTagLayer : public SuperLayer
	{
	private:
		// constants
		// default alignment, must be a power of two
		static const size_t DEFAULT_ALIGNMENT  = sizeof(U64);
		//memory guard info
		//should probably go into a debug layer?
		static const U32 HEADER_GUARD = 0xDECADEAF;
		#ifdef DEBUG_ALLOCATOR
		static const size_t MEMORY_GUARD_SIZE  = 16UL;
		#else
		static const size_t MEMORY_GUARD_SIZE  = 0UL;
		#endif
		//mainly defining what counts as a small alloc
		static const size_t MAX_SMALL_ALLOCATION_LOG2 = 8UL;
		static const size_t MAX_SMALL_ALLOCATION  = 1UL << MAX_SMALL_ALLOCATION_LOG2;
		// block header is where the large allocator stores its book-keeping information
		// it is always located in front of the payload block
		class BlockHeader 
		{
		public:
			U32 HeaderGuard;
		private:
			enum block_flags {BLOCK_USED = 1};
			BlockHeader* prevHeader;
			size_t sizeAndFlags;
			#if defined( _MSC_VER )
			#pragma warning(push)
			#pragma warning (disable : 4200) // zero sized array
			#endif
			unsigned char _padding[DEFAULT_ALIGNMENT <= sizeof(BlockHeader*) + sizeof(size_t) ? 0 : DEFAULT_ALIGNMENT - sizeof(BlockHeader*) - sizeof(size_t)];
			#if defined( _MSC_VER )
			#pragma warning(pop)
			#endif
		public:
			BlockHeader()
			{
				HeaderGuard = HEADER_GUARD;
				prevHeader = NULL;
				sizeAndFlags = 0;
			}
			size_t Size() const {return sizeAndFlags & ~3;}
			//U32 HeaderGuard() const { return HeaderGuard; }
			/*
			void SetHeaderGuard(U32 guardVal) 
			{
				HeaderGuard = guardVal;
			}
			*/
			BlockHeader* Next() const {return (BlockHeader*)((char*)Data() + Size());}
			BlockHeader* Prev() const {return prevHeader;}
			void* Data() const {return (void*)((char*)this + sizeof(BlockHeader));}
			bool Used() const {return (sizeAndFlags & BLOCK_USED) != 0;}
			void SetUsed() {sizeAndFlags |= BLOCK_USED;}
			void SetUnused() {sizeAndFlags &= ~BLOCK_USED;}
			void Unlink() {
				Next()->SetPrev(Prev());
				Prev()->SetNext(Next());
			}
			//places this header between given header and the given header's next header
			void LinkAfter(BlockHeader* link) {
				SetPrev(link);
				SetNext(link->Next());
				Next()->SetPrev(this);
				Prev()->SetNext(this);
			}
			//sets block size
			void SetSize(size_t size) {
				L_ASSERT((size & 3) == 0);
				sizeAndFlags = (sizeAndFlags & 3) | size;
			}
			//sets next alloc block
			void SetNext(BlockHeader* next) {
				L_ASSERT(next >= Data());
				SetSize((char*)next - (char*)Data());
			}
			//sets previous alloc block
			void SetPrev(BlockHeader* prev) {
				prevHeader = prev;
			}
		};
		// free node is what the large allocator uses to find free space
		// it's stored next to the block header when a block is not in use
		struct FreeNode : public RedBlackTree<FreeNode>::RBTNode
		{
			BlockHeader* GetBlock() const {return (BlockHeader*)((char*)this - sizeof(BlockHeader));}
			bool operator<(const FreeNode& rhs) const {return GetBlock()->Size() < rhs.GetBlock()->Size();}
			bool operator>(const FreeNode& rhs) const {return GetBlock()->Size() > rhs.GetBlock()->Size();}
			bool operator<(size_t size) const {return GetBlock()->Size() < size;}
			bool operator>(size_t size) const {return GetBlock()->Size() > size;}
		};

		RedBlackTree<FreeNode> freeNodeTree;
		BlockHeader* mostRecentBlock;
		//small free nodes need to be implemented

		static inline BlockHeader* getBlockHeader(void* ptr)
		{
			//header's at front of data
			return (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
		}

		//attaches a block from the free node tree
		void attachHeader(BlockHeader* header)
		{
			if(header)
			{
				L_ASSERT(header->HeaderGuard == HEADER_GUARD);
			}
			//need to get the most recent block if it exists
			if(mostRecentBlock)
			{
				BlockHeader* prevBlock = mostRecentBlock;
				L_ASSERT(prevBlock->HeaderGuard == HEADER_GUARD);
				//if(prevBlock->Size() > MAX_SMALL_ALLOCATION)
				//void* p = prevBlock->Data();
				//FreeNode* n = static_cast<FreeNode*>(p);
				//n->DEBUG_FLASH();
				freeNodeTree.Insert((FreeNode*)prevBlock->Data());
				L_ASSERT(prevBlock->HeaderGuard == HEADER_GUARD);
			}
			mostRecentBlock = header;
			if(header)
			{
				L_ASSERT(header->HeaderGuard == HEADER_GUARD);
				//this is, assuming all is going right,
				//a noncontiguous memory piece, so report it to stats
				ReportFragPiece(header->Size());
			}
		}

		//detaches a block from the free node tree
		void detachHeader(BlockHeader* header)
		{
			//report that this isn't free space anymore
			if(header)
			{
				RemoveFragPiece(header->Size());
			}

			//simple to do if the parameter's the most recent block,
			//no need to remove any data
			if(header == mostRecentBlock)
			{
				//just mark that there's no recent block now
				mostRecentBlock = NULL;
				return;
			}
			else
			{
				//otherwise, we do need to remove data
				//if(header->Size() > MAX_SMALL_ALLOCATION)
				//{
				//}
				//we'll need to reinterpret the pointer as a free node
				L_ASSERT(header->HeaderGuard == HEADER_GUARD);
				freeNodeTree.Erase((FreeNode*)header->Data());
			}
		}

		//kind of self explanatory
		void splitBlock(BlockHeader* header, size_t size)
		{
			//the block should have enough free space to do this split...
			L_ASSERT(header->Size() >= size + sizeof(BlockHeader) + sizeof(FreeNode));
			BlockHeader* newBlock = (BlockHeader*)((char*)header + size + sizeof(BlockHeader));
			newBlock->HeaderGuard = HEADER_GUARD;
			newBlock->LinkAfter(header);
			newBlock->SetUnused();
			header->HeaderGuard = HEADER_GUARD;
		}

		BlockHeader* shiftBlock(BlockHeader* header, size_t offset)
		{
			//this always sets the given block as unused,
			//so trying an offset by 0 will do very bad things if the block's
			//still being used
			L_ASSERT(offset > 0);
			//get the prior block so we can update its pointer to the
			//offset block
			BlockHeader* prevBlock = header->Prev();
			header->Unlink();
			//now that the header's not in the block chain, we can shift
			header = (BlockHeader*)((char*)header + offset);
			header->HeaderGuard = HEADER_GUARD;
			header->LinkAfter(prevBlock);
			header->SetUnused();
			return header;
		}

		//combines a given header's block and any unused neighboring headers's blocks into one bigger block
		BlockHeader* coalesceBlock(BlockHeader* header)
		{
			//I swear you better not call this on a block in use or I will be very mad
			L_ASSERT(!header->Used());
			BlockHeader* next = header->Next();
			if(!next->Used())
			{
				//remove the block from the tree
				detachHeader(next);
				//and then detach it from the header list
				next->Unlink();
			}
			//do something similar for previous node
			BlockHeader* prev = header->Prev();
			if(!prev->Used())
			{
				//remove the block from the tree
				detachHeader(prev);
				//this time, remove the HEADER (since the previous header will be covering the data
				//the passed header exists in)
				header->Unlink();
				header = prev;
			}
			header->HeaderGuard = HEADER_GUARD;
			return header;
		}
		
		//generates a block header for a given chunk of memory
		BlockHeader* addBlock(void* mem, size_t memSize)
		{
			//make a dummy block to prevent prev() = null, 
			//and allow block shifts
			BlockHeader* blockFront = (BlockHeader*)mem;
			blockFront->SetPrev(NULL);
			blockFront->SetSize(0);
			blockFront->SetUsed();

			BlockHeader* blockBack = (BlockHeader*)blockFront->Data();
			blockBack->SetPrev(blockFront);
			blockBack->SetSize(0);
			blockBack->SetUsed();

			//make the real block now
			blockFront = blockBack;
			L_ASSERT(blockFront->Used());
			//remember - the front includes the header's position
			//so the back needs to not include that space
			blockBack = (BlockHeader*)((char*)mem + memSize - sizeof(BlockHeader));
			blockBack->SetSize(0);
			blockBack->SetUsed();
			blockFront->SetUnused();
			blockFront->SetNext(blockBack);
			blockBack->SetPrev(blockFront);
			blockBack->HeaderGuard = HEADER_GUARD;
			blockFront->HeaderGuard = HEADER_GUARD;
			//this coalesce will now make the desired block
			blockFront = coalesceBlock(blockFront);
			return blockFront;
		}

		//gets a block in the free node tree greater than or equal to desired size
		BlockHeader* extractBlock(size_t size)
		{
			//try the most recent block
			BlockHeader* bestMatch = mostRecentBlock;
			//if it works, take it off the tree (since it's going to be in use)
			//and return it
			if(bestMatch && bestMatch->Size() >= size)
			{
				detachHeader(bestMatch);
				return bestMatch;
			}
			//otherwise, we'll need to search the tree itself
			FreeNode* bestMatchNode = freeNodeTree.LowerBound(size);
			//if there's no match, give up :(
			if(bestMatchNode == freeNodeTree.End())
			{
				return NULL;
			}
			//why does this "improve removal time"? check errata
			bestMatchNode = bestMatchNode->Next();
			bestMatch = bestMatchNode->GetBlock();
			detachHeader(bestMatch);
			return bestMatch;
		}

		//aligned version of extractBlock here

		BlockHeader* growTree(size_t size)
		{
			//two fences plus one fake??
			size += 3*sizeof(BlockHeader);
			//whatever the case, round this up to the next page
			size = roundUp(size, PAGE_SIZE);
			void* mem = SuperLayer::Malloc(size);
			if(mem)
			{
				return addBlock(mem, size);
			}
			return NULL;
		}

	public:
		RBTreeTagLayer() : mostRecentBlock(0){}
		inline void* Malloc(size_t size)
		{
			//size must be at least big enough to fit a free node and block header
			if(size < sizeof(FreeNode))
			{
				size = sizeof(FreeNode);
			}
			size = roundUp(size, sizeof(BlockHeader));
			//try to get an existing free block...
			BlockHeader* newBlock = extractBlock(size);
			//if we couldn't find one, we need to ask for another page
			if(!newBlock)
			{
				//grow the tree
				newBlock = growTree(size);
				//if we couldn't get a new page, NULL!
				if(!newBlock)
				{
					return NULL;
				}
			}

			//now we should have a block.
			//chop off everything beyond the node, header, and alloc
			//and make that another node
			//we can access this new node by calling Next() on the alloc's block,
			//since it will skip past the alloc's data to the next header
			
			//of course the block should be able to *fit*
			//our desired alloc
			L_ASSERT(newBlock && (newBlock->Size() >= size));
			if(newBlock->Size() >= size + sizeof(BlockHeader) + sizeof(FreeNode))
			{
				splitBlock(newBlock, size);
				attachHeader(newBlock->Next());
			}

			//once that's all done, note that this block's in use
			//and return the data portion
			newBlock->SetUsed();
			//also report the overhead of the header & node
			ReportOverhead(sizeof(BlockHeader) + sizeof(FreeNode));
			return newBlock->Data();
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			//okay, this is going to suck
			//first, we need the block's size to know what to do next
			if(size < sizeof(FreeNode))
			{
				size = sizeof(FreeNode);
			}
			size = roundUp(size, sizeof(BlockHeader));
			BlockHeader* header = getBlockHeader(ptr);
			size_t blockSize = header->Size();
			//if the new size is smaller, we need to shrink
			//that means splitting the block
			if(blockSize >= size)
			{
				if(blockSize >= size + sizeof(BlockHeader) + sizeof(FreeNode))
				{
					splitBlock(header, size);
					//try to give the block ahead the slack space
					BlockHeader* nextBlock = header->Next();
					nextBlock = coalesceBlock(nextBlock);
					attachHeader(nextBlock);
				}
				L_ASSERT(header->Size() >= size);
				return ptr;
			}

			//otherwise, we're growing
			//we need to see if the next block is free and can hold the requested size
			BlockHeader* nextBlock = header->Next();
			//if it's not free, we can't use it, obviously
			size_t nextSize = nextBlock->Used() ? 0 : nextBlock->Size() + sizeof(BlockHeader);
			if(blockSize + nextSize >= size)
			{
				L_ASSERT(!nextBlock->Used());
				detachHeader(nextBlock);
				nextBlock->Unlink();
				//now that the next block is definitely outside of the free node tree
				//and the block chain, it's just raw data that's a part of the target block
				//split any slack of this joined block
				L_ASSERT(header->Size() >= size);
				if(header->Size() >= size + sizeof(BlockHeader) + sizeof(FreeNode))
				{
					splitBlock(header, size);
					attachHeader(header->Next());
				}
				return ptr;
			}

			//if the next block won't do for whatever reason,
			//see if the prev block can work
			BlockHeader* prevBlock = header->Prev();
			//if it's not free, we can't use it, obviously
			size_t prevSize = prevBlock->Used() ? 0 : prevBlock->Size() + sizeof(BlockHeader);
			//the next block should be considered -
			//we might not have been able to use it because it couldn't provide
			//enough extra space alone
			if(blockSize + prevSize + nextSize >= size)
			{
				L_ASSERT(!prevBlock->Used());
				detachHeader(prevBlock);
				header->Unlink();
				if(!nextBlock->Used())
				{
					detachHeader(nextBlock);
					nextBlock->Unlink();
				}
				header = prevBlock;
				header->SetUsed();
				L_ASSERT(header->Size() >= size);
				void* newPtr = header->Data();
				//we need to move the memory back now so that the ptr's
				//actually pointing to the right data
				//should also prevent splitting the data when we split the slack block
				memmove(newPtr, ptr, blockSize - MEMORY_GUARD_SIZE);
				if(header->Size() >= size + sizeof(BlockHeader) + sizeof(FreeNode))
				{
					splitBlock(header, size);
					attachHeader(header->Next());
				}
				return newPtr;
			}

			//otherwise, we're really hosed, and need to make an alloc.
			void* newPtr = Malloc(size);
			if(newPtr)
			{
				//copy in the data, and free the source pointer
				memcpy(newPtr, ptr, blockSize - MEMORY_GUARD_SIZE);
				Free(ptr);
				return newPtr;
			}
			return NULL;
		}

		void attemptPurgeBlock(BlockHeader* header)
		{
			//we should only be attempting this on completely free pages
			L_ASSERT(!header->Used());
			//since this should include ALL the memory in a page,
			//the neighboring blocks better not also be free
			//(implies they can also be coalesced)
			L_ASSERT(header->Prev() && header->Prev()->Used());
			L_ASSERT(header->Next() && header->Next()->Used());
			if (header->Prev()->Prev() == NULL && header->Next()->Size() == 0)
			{
				detachHeader(header);
				char* memStart = (char*)header->Prev();
				char* memEnd = (char*)header->Data() + header->Size() + sizeof(BlockHeader);
				void* allocPoint = memStart;
				size_t size = memEnd - memStart;
				L_ASSERT(((size_t)allocPoint & (PAGE_SIZE-1)) == 0);
				L_ASSERT((size & (PAGE_SIZE-1)) == 0);
				SuperLayer::Free(allocPoint, size);

				//now we need to inform the handle system that the memory is purged
				HandleMgr::NotifyRegionPurged(memStart, size);
			}
		}

		inline void Free(void* ptr)
		{
			//since this should be a data pointer, go back by the size of a HEADER (node shouldn't exist in allocated data)
			BlockHeader* header = getBlockHeader(ptr);
			//set the block as unused, coalesce it if possible, and then reattach the block to the tree
			header->SetUnused();
			header = coalesceBlock(header);
			attachHeader(header);
			//if the block's >= page size, then we should be able to purge the block
			//(block header should be @ page start when this is the case)
			//need to be more elaborate about this -
			//a bunch of things cause an access violation w/ VirtualFree
			/*if(header->Size() > PAGE_SIZE)
			{
				if((size_t)header & (PAGE_SIZE-1) == 0)
				{
					attemptPurgeBlock(header);
				}
				else
				{
					Log::D(String("Failed to free address ") + StrFromPtr(header) + ", size " + header->Size());
				}
			}
			else
			{
				
			}*/
			//and note that the overhead's gone
			//(Free node counts for the stats, it's overhead to the alloc being freed)
			RemoveOverhead(sizeof(BlockHeader) + sizeof(FreeNode));
			
		}

		inline size_t GetSize(void* ptr)
		{
			//since this should be a data pointer, go back by the size of a header
			BlockHeader* header = getBlockHeader(ptr);
			//assuming this is valid, read the pointer as a header and return the size data
			if(header->HeaderGuard == HEADER_GUARD)
			{
				return header->Size();
			}
			return 0;
		}

		inline void Purge()
		{
			//purge the most recently used block?
			attachHeader(NULL);
			//iterate through the free nodes
			//for nodes larger than a page
			//since the node may also have used dummy headers to get desired alignments or sizes,
			//include that in the threshold value
			const static size_t pageSize = PAGE_SIZE-(3*sizeof(BlockHeader))-sizeof(FreeNode);
			FreeNode* currNode = freeNodeTree.LowerBound(pageSize);
			FreeNode* end = freeNodeTree.End();
			while(currNode != end)
			{
				BlockHeader* currHeader = currNode->GetBlock();
				currNode = currNode->Succ();
				attemptPurgeBlock(currHeader);
			}

			//and then, uh, re-purge the MRblock?
			attachHeader(NULL);
		}
	};

	//Aligned allocation layer?
	//Simple idea - allocate a little more memory than needed,
	//shift up pointer until it's aligned, return that pointer
	//Or would this be more useful as a direct feature in Allocator?
	//"alignment" is in bits.
	template<size_t alignment, class SuperLayer> class AlignedLayer : public SuperLayer
	{
	public:
		inline void* Malloc(size_t size)
		{
			//Simple idea - allocate a little more memory than needed,
			//enough to ensure a section in the alloc's aligned.
			//Also include a tiny bit of space to store a pointer to the raw alloc
			size_t allocSize = size + SLACK;
			void* rawMem = SuperLayer::Malloc(allocSize);
			//Shift up pointer until it's aligned, return that pointer.
			void **res = (void**)((size_t)(rawMem+SLACK) & ~(alignment - 1));
			//Store the original pointer right behind the result alloc
			res[-1] = rawMem;
			return res;
		}

		inline void Free(void* ptr, size_t size = 0)
		{
			//Trivial thanks to the bookkeeping pointer -
			//Move back by a pointer, and free that address
			SuperLayer::Free(((void**)ptr)[-1], size);
		}

		inline void* Realloc(void* ptr, size_t size)
		{
			Free(ptr);
			return Malloc(size);
		}
	};
}
