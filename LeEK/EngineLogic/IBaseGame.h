#pragma once
#include "Time/GameTime.h"

namespace LeEK
{
	class IBaseGame
	{
	public:
		virtual bool Startup() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(const GameTime& time) = 0;
		virtual void Draw(const GameTime& time) = 0;
		virtual void Run() = 0;
	};
}