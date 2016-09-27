#pragma once
#include "Memory/Allocator.h"
#include "Memory/STLAllocHook.h"
#include <map>
#include <list>
#include <stack>
#include <vector>
#include <deque>

namespace LeEK
{
	template<typename A, typename B>
	class Pair : public std::pair<A, B>
	{
	public:
		Pair(A a, B b)
		{
			first = a;
			second = b;
		}
		Pair()
		{
			first = A();
			second = B();
		}
	};

	template<typename keyT, typename valT>
	class Map : public std::map<keyT, valT, std::less<keyT>, STLAllocHook<std::pair<const keyT, valT>>>
	{
	public:
		Map() {}
		~Map() {}
	};

	template<typename T>
	class List : public std::list<T, STLAllocHook<T>>
	{
	public:
		List() {}
		~List() {}
	};
	
	template<typename T>
	class Vector : public std::vector<T, STLAllocHook<T>>
	{
	public:
		Vector() {}
		~Vector() {}
	};
	

	/*
	//This version is a reimplementation of the vector class, to allow debugging
	//memory issues that may arise.
	template<typename T>
	class Vector : public std::vector<T, STLAllocHook<T>>
	{
	private:
		//How much the backing array should grow by if filled.
		//Also decides when the array can be shrunk, if that is implemented.
		const static size_t GROWTH_RATE = 2;
		//The smallest capacity allowed.
		const static size_t MIN_CAPACITY = 8;
		//Some basic things are needed:
		//	* reserve()
		//	* operator[]
		//	* push_back()
		//	* size()
		//	* capacity()
		//	* empty()
		//	* iterator methods. Implies we need implementations of
		//	iterator and const_iterator.

		size_t arraySize;
		size_t arrayCapacity;
		T* data;

		void copyData(T* newData) const
		{
			for(size_t i = 0; i < arraySize; ++i)
			{
				new((void*)(newData + i)) T(data[i]);
			}
		}

		void grow()
		{
			//Update capacity.
			size_t newCapacity = arrayCapacity > 0 ? 
												arrayCapacity * GROWTH_RATE :
												MIN_CAPACITY;
			reserve(newCapacity);
		}

		void deleteData()
		{
			//Call destructor on all elements.
			for(size_t i = 0; i < arraySize; ++i)
			{
				data[i].~T();
			}
			//No more elements in use; reset size counter.
			arraySize = 0;
		}
	public:
		typedef T* iterator;
		typedef const T* const_iterator;

		Vector()
		{
			arraySize = 0;
			arrayCapacity = 0;
			data = NULL; //Allocator::STLMalloc(arrayCapacity * sizeof(T));
		}
		Vector(const Vector& other)
		{
			arraySize = other.arraySize;
			arrayCapacity = other.arrayCapacity;
			if(arrayCapacity > 0)
			{
				data = static_cast<T*>(Allocator::STLMalloc(arrayCapacity * sizeof(T)));
				other.copyData(data);
			}
			else
			{
				data = NULL;
			}
		}
		~Vector()
		{
			if(data)
			{
				Allocator::STLFree(data, arrayCapacity * sizeof(T));
			}
		}

		size_t size() const
		{
			return arraySize;
		}

		size_t capacity() const
		{
			return arrayCapacity;
		}

		iterator begin()
		{
			return data;
		}

		iterator end()
		{
			return (data + arraySize);
		}

		const_iterator cbegin() const
		{
			return data;
		}

		const_iterator cend() const
		{
			return (data + arraySize);
		}

		void push_back(const T& val)
		{
			//if we're over capacity, grow the array.
			if(arraySize >= arrayCapacity)
			{
				grow();
			}
			//Now copy over the inserted value. This can be done with placement new.
			new((void*)(data + arraySize)) T(val);
			//Update the array size.
			++arraySize;
			return;
		}

		void erase(iterator at)
		{
			if(at != end())
			{
				//Delete the initial value.
				//Move all values after it up.
				at->~T();
				while(at + 1 != end())
				{
					//Move all values after it up.
					new (at) T(*(at + 1));
					++at;
				}

				--arraySize;
				//Presumably if the array size is too small, we'd shrink it here.
			}
		}

		void erase(size_t at)
		{
			if(at < arraySize)
			{
				erase(begin() + at);
			}
		}

		void reserve(size_t resrvSize)
		{
			if(resrvSize <= arrayCapacity)
			{
				return;
			}
			//we need to allocate and swap over a new, larger array.
			size_t oldCapacity = arrayCapacity;
			//Update capacity.
			arrayCapacity = resrvSize;

			//Now allocate the needed size.
			T* newData = static_cast<T*>(Allocator::STLMalloc(arrayCapacity * sizeof(T)));
			//Default initialize all elements.
			for(size_t i = 0; i < arrayCapacity; ++i)
			{
				new((void*)(newData + i)) T;
			}
			//Copy data over, if needed.
			if(data)
			{
				copyData(newData);
				deleteData();
			}
			//New array's ready. Delete the old one
			//and replace it with the new.
			T* oldData = data;
			data = newData;
			if(oldData)
			{
				Allocator::STLFree(oldData, oldCapacity * sizeof(T));
			}
		}

		T& operator[](size_t idx)
		{
			return data[idx];
		}

		const T& operator[](size_t idx) const
		{
			return data[idx];
		}
	};
	*/

	template<typename T>
	class Stack : public std::stack<T, std::deque<T, STLAllocHook<T>>>
	{
	public:
		Stack() {}
		~Stack() {}
	};

	template<typename T>
	class Deque : public std::deque<T, STLAllocHook<T>>
	{
	public:
		Deque() {}
		~Deque() {}
	};
}