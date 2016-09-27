#include "TestGame.h"
#include <Logging/Log.h>

using namespace LeEK;

TestGame::~TestGame(void)
{
}

bool TestGame::Startup()
{
	if(!test)
	{
		Log::E("No test set!");
		return false;
	}
	//if a test is assigned, enable or disable window display as needed.
	showWnd = test->ShouldShowWnd();
	if(!Game::Startup())
	{
		return false;
	}
	//if additional debug config's needed, load it here
	//Logging's global, set its verbosity here
	//Log::SetVerbosity(Log::Verbosity::DEBUG);
	//Stats().SetShouldPrint(true);
	if(!test->PreStartup(this))
	{
		//press any key
		getchar();
		return false;
	}
	if(!test->Startup(this))
	{
		//press any key
		getchar();
		return false;
	}
	return true;
}

void TestGame::Shutdown()
{
	test->Shutdown(this);
	Game::Shutdown();
	if(Platform())
	{
		Stats().PrintStats();
	}
	//press any key
	std::getchar();
}

void TestGame::PreOS()
{
	test->PreOS(this);
}

void TestGame::Update(const GameTime& time)
{
	test->Update(this, time);
}

void TestGame::Draw(const GameTime& time)
{
	test->Draw(this, time);
}