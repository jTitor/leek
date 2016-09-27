#pragma once
#include "IThreadClient.h"
#include "Constants/AllocTypes.h"
#include <thread>
#include <mutex>

//note to self - do not permadelete when in /Dev folder
//how did we do this? clearly, it's STL, but not clear on the exact implementation
//becoming rather apparent I had no idea how mutexes and locks work...
//unclear how thread test is working, since mutex is synchronizing via thread joining without using any lock at all

namespace LeEK
{
	class Thread
	{
	private:
		std::thread* t;
		//void (*IThreadClient::client)(void);
		IThreadClient* cli;
		
		Thread(Thread const& copy) { L_ASRT_FORBIDDEN(); }
		Thread& operator=(Thread const& copy) { L_ASRT_FORBIDDEN(); }
	public:
		Thread(IThreadClient* c)  : t(), cli(c)
		{ 
			//cli = c;
			//client = &IThreadClient::Run;
		}
		~Thread()
		{
			//In case we forgot to join or detach, try joining.
			if(t->joinable())
			{
				t->join();
			}
		}
		void Start()
		{	//std::thread newThread(client); 
			t = CustomNew<std::thread>(THREAD_ALLOC, "ThreadAlloc", &IThreadClient::Run, cli);//new std::thread(&IThreadClient::Run, cli);
		}//&newThread; }
		void Join() { t->join(); }
		void Detatch() { t->detach(); }
	};

	class ILock
	{
	};

	class IMutex
	{
	//public:
	//	virtual ILock GetLock() = 0;
	};

	class Mutex : public IMutex
	{
	private:
		std::mutex m;
		Mutex(const Mutex& original) {}
	public:
		Mutex() {}
		std::mutex& GetMutex() { return m; }
		//ILock GetLock();
	};

	class Lock : public ILock
	{
	private:
		std::lock_guard<std::mutex> l;
	public:
		Lock(Mutex& mutHolder) : l(mutHolder.GetMutex())
		{
		}
	};
}