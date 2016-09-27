#pragma once
#include <EngineLogic/Game.h>
#include "TestBase.h"
namespace LeEK
{
	class TestGame : public Game
	{
	private:
		TestBase* test;
	public:
		TestGame(TestBase* gameTest) : test(gameTest) {}
		~TestGame(void);
		bool ShouldShowWnd() { return showWnd; }
		void SetShowWind(bool val) { showWnd = val; }
		bool ShouldDisplayStats() { return stats.ShouldPrint(); }
		void SetShouldDisplayStats(bool val) { stats.SetShouldPrint(val); }
		bool Startup();
		void Shutdown();
		void PreOS();
		void Update(const GameTime& time);
		void Draw(const GameTime& time);
	};
}
