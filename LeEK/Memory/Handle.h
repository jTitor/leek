#pragma once
#include "Datatypes.h"
#include "DebugUtils/Assertions.h"
#include "Memory/Allocator.h"

namespace LeEK
{
	/**
	*	Handle definitions.
	*	A handle in LeEK is a unsigned integer that can be used to find a pointer;
	*	you can change what pointer the handle refers to as well.
	*	Only positive handles are valid, so you can test for validity via (handle == 0) or just (handle).
	*/

	typedef U32 Handle;
	const Handle INVALID_HND = 0;
	const Handle MAX_HND = ~0;

	namespace HandleMgr
	{
		void* GetPointer(const Handle& hnd);
		//Handle GetHandle(const void* ptr);
		Handle RegisterPtr(void* ptr);
		Handle FindHandle(void* ptr);
		void DeleteHandle(const Handle& hnd);
		void RemoveHandle(const Handle& hnd);
		void RemovePtr(void* ptr);
		void MoveHandle(const Handle& hnd, void* newPtr);
		//used when heap has been purged; 
		//all handles with pointers within the specified region are destroyed
		void NotifyRegionPurged(void* regionStart, size_t regionSize);
	}


	template<typename T>
	class TypedHandle;

	template<typename T>
	class TypedArrayHandle;


	//templated functions
	namespace HandleMgr
	{
		template<typename T>
		T* GetPointer(const Handle& hnd) { return (T*)GetPointer(hnd); }
		template<typename T>
		TypedHandle<T> RegisterPtr(T* ptr)
		{
			Handle hnd = RegisterPtr((void*)ptr);
			return TypedHandle<T>(hnd);
		}
		template<typename T>
		void DeleteHandle(const Handle& hnd)
		{ 
			CustomDelete(GetPointer<T>(hnd));
			RemoveHandle(hnd);
		}
		template<typename T>
		void DeleteHandle(const TypedHandle<T>& hnd) { DeleteHandle<T>(hnd.GetHandle()); }
		//might be a bit of a pain, but it's sematically important
		//if you really wanted to circumvent this you could just cast anything to void*
		//and then to T* anyway
		template<typename T>
		void MoveHandle(const TypedHandle<T>& hnd, T* newPtr) { MoveHandle(hnd.GetHandle(), newPtr); }
		//array deleter
		template<typename T>
		void DeleteArrayHandle(const TypedArrayHandle<T>& hnd)
		{ 
			CustomArrayDelete<T>(GetPointer<T>(hnd.GetHandle()));
			RemoveHandle(hnd.GetHandle());
		}
	}

	//wrapper enabling typed dereferencing for Handles.
	template<typename T>
	class TypedHandle
	{
	protected:
		Handle hnd;
#if defined(_DEBUG) || defined(RELDEBUG)
		T* ptr;
#endif
	public:
		TypedHandle(Handle handle) : hnd(handle)
		{
#if defined(_DEBUG) || defined(RELDEBUG)
			ptr = HandleMgr::GetPointer<T>(hnd);
#endif
		}
		TypedHandle() : hnd(0)
		{
#if defined(_DEBUG) || defined(RELDEBUG)
			ptr = NULL;
#endif
		}
		~TypedHandle() {}
		inline Handle GetHandle() const { return hnd; }
		const T* operator->() const 
		{ 
			T* res = HandleMgr::GetPointer<T>(hnd);
			L_ASSERT(res && "Attempted to dereference null handle!");
			return res;
		}
		const T& operator*() const
		{ 
			T* res = HandleMgr::GetPointer<T>(hnd);
			L_ASSERT(res && "Attempted to dereference null handle!");
			return *res;
		}
		T* operator->()
		{ 
			T* res = HandleMgr::GetPointer<T>(hnd);
			L_ASSERT(res && "Attempted to dereference null handle!");
			return res;
		}
		T& operator*()
		{ 
			T* res = HandleMgr::GetPointer<T>(hnd);
			L_ASSERT(res && "Attempted to dereference null handle!");
			return *res;
		}
		T* Ptr() { return HandleMgr::GetPointer<T>(hnd); }
		const T* Ptr() const { return HandleMgr::GetPointer<T>(hnd); }
		operator int() const { return hnd; }
	};
	
	//TypedArrayHandle?
	template<typename T>
	class TypedArrayHandle : public TypedHandle<T>
	{
	public:
		TypedArrayHandle(Handle handle) : TypedHandle<T>(handle) {}
		~TypedArrayHandle() {}
		const T& operator[](U32 index) const { return HandleMgr::GetPointer<T>(TypedHandle<T>::hnd)[index]; }
		T& operator[](U32 index) { return HandleMgr::GetPointer<T>(TypedHandle<T>::hnd)[index]; }
	};
}
