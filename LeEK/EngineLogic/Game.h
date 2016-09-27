#pragma once
#include "EngineLogic/IBaseGame.h"
#include "Platforms/IPlatform.h"
#include "Stats/StatMonitor.h"
#include "Input/Input.h"
#include "Platforms/IPlatform.h"

namespace LeEK
{
	class Game : public IBaseGame
	{
	protected:
		StatMonitor stats;
		GameTime time;
		IPlatform* plat;
		TypedHandle<InputManager> input;
		GfxWrapperHandle graphics;
		F32 timeSincePurgeMs;
		bool showWnd;
		bool running;
	private:
		bool initPlatform();
		void shutdownPlatform();
		void initGfx(RendererType type, IPlatform* plat);
		bool updateOS();
		//these do pre and post prep work for their respective
		//update and draw functions, in addition to calling
		//Update() and Draw().
		void fullUpdate(const GameTime& time);
		void fullDraw(const GameTime& time);
		void updateAllocator(const GameTime& time);
	public:
		Game(void);
		~Game(void);
#pragma region Properties
		inline StatMonitor& Stats() { return stats; }
		inline GameTime& Time() { return time; }
		inline IPlatform* Platform() { return plat; }
		inline GfxWrapperHandle GfxWrapper() { return graphics; }
		inline TypedHandle<InputManager> Input() { return input; }
#pragma endregion
		virtual bool Startup();
		virtual void Shutdown();
		virtual void Update(const GameTime& time) = 0;
		virtual void Draw(const GameTime& time) = 0;
		virtual void PreOS();
		void Run();
		void Quit() { running = false; }
	};
}