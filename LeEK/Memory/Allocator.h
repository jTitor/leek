#pragma once
#include "Datatypes.h"
#include "Constants/AllocTypes.h"
#include "FileManagement/DataStream.h"
#include "DebugUtils/Assertions.h"

//Welcome to the Thunderdome.

//global tagged new from HPHA
//according to source, the custom_tag struct's needed
//so that memory can be freed if a constructor throws an exception
//(lookup what they mean by that...)
struct custom_tag {};
extern void* operator new(size_t, void* ptr, custom_tag);
extern void* operator new[](size_t, void* ptr, custom_tag);
extern void operator delete(void* ptr, void*, custom_tag);
extern void operator delete[](void* ptr, void*, custom_tag);

//template array info deducers,
//from http://molecularmusings.wordpress.com/2011/07/07/memory-system-part-2/
template <class T>
struct TypeAndCount
{
};

//specialization ensures you can't call LArrayNew with a non-array
template <class T, size_t N>
struct TypeAndCount<T[N]>
{
  typedef T Type;
  static const size_t Count = N;
};

//alternate thing is to use defines
//new must be done with placement new, since we can't be sure what the constructor is initially
#define LNew(type, allocType, desc) new(Allocator::_CustomMalloc(sizeof(type), allocType, desc, __FILE__, __LINE__)) type
#define LDelete(ptr) CustomDelete(ptr)
//array is simpler, as only the default constructor can be called
//with array new
#define LArrayNew(type, count, allocType, desc) CustomArrayNew<type>(count, allocType, desc, __FILE__, __LINE__)//CustomArrayNew<TypeAndCount<type[count]>::Type>(TypeAndCount<type>::Count, allocType, desc, __FILE__, __LINE__)
#define LArrayDelete(ptr) CustomArrayDelete(ptr)

namespace LeEK
{
	//nothing to handle categories, though
	//create a subsystem for that?

	struct TagDesc
	{
		const static U32 MAX_STR_LEN = 128;
		const static U32 TAG_GUARD = 0xbadd00d5;
		char TagName[MAX_STR_LEN];	//Plaintext name of category
		U32 CRC;			//CRC of tag's string name. For indexing into hashtable
		U32 Size;			//TOTAL size of allocs in category
		U32 Category;
		//TagDesc* Next;
	public:
		static TagDesc* Register(U32 category, const char* tagName, size_t size);
	};

	struct AllocDesc
	{
		const static U32 MAX_STR_LEN = 128;
		void* Ptr;			//data address
		U32 Size;
		U32 Line;
		const char* File;
		TagDesc* Tag;		//data on alloc category
		//AllocDesc* Next;	//next alloc in hashtable
	};

	//provides global allocation functions.
	//also has global state for its allocation tracker -
	//by default, set to not use verbose allocation dumps
	class Allocator
	{
	private:
	public:
		Allocator(void);
		~Allocator(void);
		static void* _CustomMalloc(size_t size, U32 allocType, const char* desc, const char* file, U32 line);
		static void* _CustomRealloc(void* target, size_t newSize, const char* file, U32 line);
		static void _CustomFree(void* target);
		//Alignment values must be a power of 2.
		static void* _AlignedMalloc(size_t size, size_t alignment, U32 allocType, const char* desc, const char* file, U32 line);
		static void* _AlignedRealloc(void* target, size_t newSize, size_t alignment, const char* file, U32 line);
		static void _AlignedFree(void* target);
		static void* BulletMalloc(size_t size);
		//{
		//	return _CustomMalloc(size, 1, "BulletAlloc", "BulletLibrary", 0);
		//}
		static void BulletFree(void* target);
		static void* STLStrMalloc(size_t size);
		static void STLStrFree(void* target);
		static void* STLMalloc(size_t size);
		static void STLFree(void* target, size_t freedSize);
		//returns all unused memory to the OS.
		static void Purge();
		static void SetVerboseDump(bool val);
		static void DumpAllocsCSV(const char* path);
		static void WriteAllocsCSV(DataStream* file);
		static const char* FindAllocSummary();
		/**
		Returns total memory used by engine allocations, in megabytes.
		*/
		static F64 TotalMemoryAllocated();
		static F64 PeakMemoryAllocated();
	};

	//shortcut defines
	//may not be platform independent; check this when we have free time!
	#define LMalloc(SIZE, TYPE, DESC) Allocator::_CustomMalloc(SIZE, TYPE, DESC, __FILE__, __LINE__)
	#define LRealloc(PTR, SIZE) Allocator::_CustomRealloc(PTR, SIZE, __FILE__, __LINE__)
	#define LFree(PTR) Allocator::_CustomFree(PTR)
	#define LAlignedMalloc(SIZE, ALIGN, TYPE, DESC) Allocator::_AlignedMalloc(SIZE, ALIGN, TYPE, DESC, __FILE__, __LINE__)
	#define LAlignedRealloc(PTR, SIZE, ALIGN) Allocator::_AlignedRealloc(PTR, SIZE, ALIGN, __FILE__, __LINE__)
	#define LAlignedFree(PTR) Allocator::_AlignedFree(PTR)

	//template overrides from HPHA

	//templated new
	//unfortunately, VS2012 doesn't do varadic args out of box,
	//so we'll have to default to a bunch of overloads
	//for up to 4 constructor params

	//original varadic version commented
	/*
	template<typename T, typename ...Args> 
	inline T* CustomNew(U32 allocType, const char* desc, Args& ...args)
	{
		void* p = Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		//note the placement new here
		return new (p, custom_tag()) T(args);
	}
	*/

	template<class T> 
	inline T* CustomNew(U32 allocType, const char* desc)
	{
		void* p =  Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		return new (p) T();//, custom_tag()) T();
	}

	//uncomment this if you're using the macros
	template<class T> 
	inline T* CustomArrayNew(size_t count, U32 allocType, const char* desc, char* file, U32 line)
	{
		//check for attempted integer overflow
		if(count > (size_t)-1 / sizeof(T))
		{
			L_ASSERT(false && "Array count overflow = game dead");
			return NULL;
		}
		size_t totalSize = sizeof(T)*count + sizeof(U32);
		void* p =  Allocator::_CustomMalloc(totalSize, allocType, desc, file, line);
		//in the back of the main data, encode element count data.
		size_t* countData = (size_t*)p;
		*countData = count;
		//now move up by size_t to get the data pointer
		countData++;
		//that's not all - also need to call allocator on all of these, I guess.
		//ReportOverhead(sizeof(size_t));
		return new ((void*)countData) T[count];//, custom_tag()) T[count];
	}

	template<class T> 
	inline T* CustomArrayNew(size_t count, U32 allocType, const char* desc)
	{
		//check for attempted integer overflow
		if(count > (size_t)-1 / sizeof(T))
		{
			L_ASSERT(false && "Array count overflow = game dead");
			return NULL;
		}
		size_t totalSize = sizeof(T)*count + sizeof(size_t);
		void* p =  Allocator::_CustomMalloc(totalSize, allocType, desc, __FILE__, __LINE__);
		//in the back of the main data, encode element count data.
		size_t* countData = (size_t*)p;
		*countData = count;
		//now move up by size_t to get the data pointer
		countData++;
		const T* const pastEnd = ((T*)p) + count;
		T* asData = (T*)p;
		while(asData < pastEnd)
		{
			new(asData++) T;
		}
		//return new ((void*)countData) T[count];
		return (asData-count);
	}

	template<class T, class P1> 
	inline T* CustomNew(U32 allocType, const char* desc, const P1& p1)
	{
		void* p =  Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		return new (p) T(p1);//, custom_tag()) T(p1);
	}

	template<class T, class P1, class P2> 
	inline T* CustomNew(U32 allocType, const char* desc, const P1& p1, const P2& p2)
	{
		void* p =  Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		return new (p) T(p1, p2);//, custom_tag()) T(p1, p2);
	}

	template<class T, class P1, class P2, class P3> 
	inline T* CustomNew(U32 allocType, const char* desc, const P1& p1, const P2& p2, const P3& p3)
	{
		void* p =  Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		return new (p) T(p1, p2, p3);//, custom_tag()) T(p1, p2, p3);
	}

	template<class T, class P1, class P2, class P3, class P4> 
	inline T* CustomNew(U32 allocType, const char* desc, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
	{
		void* p =  Allocator::_CustomMalloc(sizeof(T), allocType, desc, __FILE__, __LINE__);
		return new (p) T(p1, p2, p3, p4);//, custom_tag()) T(p1, p2, p3, p4);
	}

	inline void CustomDelete(char* ptr)
	{
		if(ptr)
		{
			Allocator::_CustomFree(ptr);
		}
	}


	inline void CustomDelete(void* ptr)
	{
		if(ptr)
		{
			Allocator::_CustomFree(ptr);
		}
	}


	inline void CustomDelete(wchar_t* ptr)
	{
		if(ptr)
		{
			delete ptr;
			Allocator::_CustomFree(ptr);
		}
	}

	template<class T> 
	inline void CustomDelete(T* ptr)
	{
		if(ptr)
		{
			//stupid - should really do that polymorphic check!
			//void* basePtr = dynamic_cast<void*>(ptr);
			ptr->~T();
			Allocator::_CustomFree(ptr);
		}
	}

	inline void CustomArrayDelete(void* ptr)
	{
		if(ptr)
		{
			//no need to get the count data here,
			//there's no indication of type

			//remember to free the ENTIRE pointer, including the count tag.
			Allocator::_CustomFree(((size_t*)ptr)-1);
		}
	}

	template<class T> 
	inline void CustomArrayDelete(T* ptr)
	{
		if(ptr)
		{
			//get the count data
			size_t numMembers = ((size_t*)ptr)[-1];
			//L_ASSERT(numMembers && "no members in array!");
			//now destruct in reverse order (C++ standard, apparently)
			//starting from numMembers rather than (numMembers-1) to avoid underflow
			for(U32 i = numMembers; i > 0 ; --i)
			{
				ptr[i-1].~T();
			}
			Allocator::_CustomFree(((size_t*)ptr)-1);
		}
	}

	template<class T> 
	inline void CustomArrayDelete(T** ptr)
	{
		if(ptr)
		{
			//get the count data
			size_t numMembers = ((size_t*)ptr)[-1];
			//L_ASSERT(numMembers && "no members in array!");
			//now destruct in reverse order (C++ standard, apparently)
			//starting from numMembers rather than (numMembers-1) to avoid underflow
			for(U32 i = numMembers; i > 0 ; --i)
			{
				if(ptr[i-1])
				{
					ptr[i-1]->~T();
				}
			}
			Allocator::_CustomFree(((size_t*)ptr)-1);
		}
	}

	//platform-specific allocation functions
	//generally provide aligned allocations
	//TODO: don't appear to be used?
	//OSVirtualLayer::osVir*() seem to be ones in use
	inline void* OSVirAlloc(void* desiredAddr, size_t size);
	inline void OSVirFree(void* ptr, size_t size = 0);
}
