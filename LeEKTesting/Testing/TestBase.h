#pragma once
#include <Time/GameTime.h>
#include <EngineLogic/Game.h>
#include <Logging/Log.h>

namespace LeEK
{
	//Interface for testing modules.
	//Not every test needs to enter the update loop - for those,
	//you can have them halt before the loop by returning false from Startup().
	class TestBase
	{
	protected:
		GfxWrapperHandle gfx;
		GameTime* time;
		bool showWnd;
	public:
		TestBase(void) : showWnd(true)
		{
			Log::V("In TestBase constructor");
		}
		~TestBase(void)
		{
			Log::V("In TestBase destructor");
		}
		bool ShouldShowWnd() { return showWnd; }
		virtual bool Startup(Game* game) { return false; }
		virtual bool PreStartup(Game* game) 
		{
			Log::V("In TestBase.Startup()");
			//get graphics wrapper
			gfx = game->GfxWrapper();
			if(!gfx)
			{
				Log::W("TestBase: Couldn't find graphics wrapper in game!");
				return false;
			}
			time = &(game->Time());
			if(!time)
			{
				Log::W("TestBase: Couldn't find timer in game!");
				return false;
			}
			return true;
		}
		virtual void Shutdown(Game* game)
		{
			Log::V("In TestBase.Shutdown()");
		}
		virtual void PreOS(Game* game) {}
		virtual void Update(Game* game, const GameTime& time) {}
		virtual void Draw(Game* game, const GameTime& time) {}
	};
}
