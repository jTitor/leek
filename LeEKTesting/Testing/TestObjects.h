#pragma once
#include <Datatypes.h>
#include <Logging/Log.h>

namespace LeEK
{
	class TestParent
	{
	protected:
		U32 parentData;
	public:
		TestParent() { Log::D("TestParent: in constructor"); }
		~TestParent() { Log::D("TestParent: in destructor"); }
		virtual void TestFunc() { Log::D("TestParent: called TestFunc"); }
	};

	class TestChild : public TestParent
	{
	protected:
		U16 childData;
	public:
		TestChild() { Log::D("TestChild: in constructor"); }
		~TestChild() { Log::D("TestChild: in destructor"); }
		void TestFunc()
		{ 
			TestParent::TestFunc();
			Log::D("TestChild: called TestFunc");
		}
	};
}