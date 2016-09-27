#pragma once
#include "Memory/Allocator.h"
#include <memory>
#include <iostream>

namespace LeEK
{
//custom allocator to link STL objects into allocator system
	template <typename T> class STLAllocHook
	{
	public:
		typedef size_t    size_type;
		typedef std::ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		STLAllocHook() {}
		STLAllocHook(const STLAllocHook&) {}
		//for rebinds
		template <typename U>
		STLAllocHook(const STLAllocHook<U>&) {}
		~STLAllocHook() {}

		template <typename U>
		struct rebind
		{
			typedef STLAllocHook<U> other;
		};

		inline pointer address(reference x) const
		{
			return &x;
		}

		inline const_pointer address(const_reference x) const
		{
			return &x;
		}

		pointer allocate(size_type size, typename std::allocator<void>::const_pointer hint = 0)
		{
			//void* vaddress = Allocator::STLMalloc(size * sizeof(T));
			void* vaddress = Allocator::_CustomMalloc(size*sizeof(T), STLHOOK_ALLOC, "STLAlloc", "STL", 0);
			if(!vaddress)
			{
				throw std::bad_alloc();
			}
			return static_cast<pointer>(vaddress);
		}

		inline void deallocate(pointer p, size_type n)
		{
			//do whatever, I guess
			//std::cout << "dealloc\n";
			//Allocator::STLFree(p, n * sizeof(T));
			Allocator::_CustomFree(p);
		}

		size_type max_size() const
		{
			return size_type(-1);//static_cast<size_type>(-1) / sizeof(value_type);
		}

		void construct(pointer p, const value_type& val)
		{
			new (p) value_type(val);
		}

		void destroy(pointer p)
		{
			//std::cout << "destructor\n";
			p->~T();
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}

		/// Copy
		STLAllocHook<T>& operator=(const STLAllocHook&)
		{
			return *this;
		}
		/// Copy with another type
		template<typename U>
		STLAllocHook& operator=(const STLAllocHook<U>&) 
		{
			return *this;
		}
	};

	//the hook doesn't have any local state, so any instances are effectively the same
	template <typename T>
	inline bool operator==(const STLAllocHook<T>& lhs, const STLAllocHook<T>& rhs) { return true; }
	template <typename T>
	inline bool operator!=(const STLAllocHook<T>& lhs, const STLAllocHook<T>& rhs) { return false; }

	//specialization to handle void pointers
	template <> class STLAllocHook<void>// : public std::allocator
	{
	public:
      typedef void*        pointer;
      typedef const void*  const_pointer;
      typedef void         value_type;

	  //for rebinds
	  template <typename U>
	  struct rebind
	  {
		  typedef STLAllocHook<U> other;
	  };
	};

	template <typename T> class STLStrAllocHook
	{
	public:
		typedef size_t    size_type;
		typedef std::ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		STLStrAllocHook() {}
		STLStrAllocHook(const STLStrAllocHook&) {}
		//for rebinds
		template <typename U>
		STLStrAllocHook(const STLStrAllocHook<U>&) {}
		~STLStrAllocHook() {}

		template <typename U>
		struct rebind
		{
			typedef STLStrAllocHook<U> other;
		};

		inline pointer address(reference x) const
		{
			return &x;
		}

		inline const_pointer address(const_reference x) const
		{
			return &x;
		}

		pointer allocate(size_type size, typename std::allocator<void>::const_pointer hint = 0)
		{
			(void)hint; // unused
			void* vaddress = Allocator::STLStrMalloc(size*sizeof(T));
			if(!vaddress)
			{
				throw std::bad_alloc();
			}
			return static_cast<pointer>(vaddress);
		}

		inline void deallocate(pointer p, size_type)
		{
			//do whatever, I guess
			Allocator::STLStrFree(p);
		}

		size_type max_size() const
		{
			return size_type(-1);//static_cast<size_type>(-1) / sizeof(value_type);
		}

		void construct(pointer p, const T& val)
		{
			new ((T*)p) T(val);
		}

		void destroy(pointer p)
		{
			//std::cout << "destructor\n";
			p->~T();
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}

		/// Copy
		STLStrAllocHook<T>& operator=(const STLStrAllocHook&)
		{
			return *this;
		}
		/// Copy with another type
		template<typename U>
		STLStrAllocHook& operator=(const STLStrAllocHook<U>&) 
		{
			return *this;
		}
	};

	//the hook doesn't have any local state, so any instances are effectively the same
	template <typename T>
	inline bool operator==(const STLStrAllocHook<T>&, const STLStrAllocHook<T>&) { return true; }
	template <typename T>
	inline bool operator!=(const STLStrAllocHook<T>&, const STLStrAllocHook<T>&) { return false; }

	//specialization to handle void pointers
	template <> class STLStrAllocHook<void>// : public std::allocator
	{
	public:
      typedef void*        pointer;
      typedef const void*  const_pointer;
      typedef void         value_type;

	  //for rebinds
	  template <typename U>
	  struct rebind
	  {
		  typedef STLStrAllocHook<U> other;
	  };
	};

	template<typename T>
	struct STLDeleter
	{
		void operator()(T* ptr) const
		{
			CustomDelete(ptr);
		}
	};

	template<typename T>
	std::shared_ptr<T> GetSharedPtr(T* basePtr)
	{
		return std::shared_ptr<T>(basePtr, STLDeleter<T>());
	}
}
