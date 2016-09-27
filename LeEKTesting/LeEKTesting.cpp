//entry point for app.
#include <DebugUtils/CompilerHooks.h>
#include "StdAfx.h"
#include <Logging/Log.h>
#include <Constants/LogTags.h>
#include <Constants/AllocTypes.h>
#include <Platforms/IPlatform.h>
#include <Platforms/Win32Platform.h>
#include "Testing/TestGame.h"
#include "Testing/Tests/TestModules.h"
//#include <vld.h>
using namespace LeEK;

int main()
{
	Log::SetVerbosity(Log::VERB);
	Log::SetBufferEnabled(false);
	Tests::SceneTest t;
	TestGame testGame(&t);
	testGame.SetShouldDisplayStats(false);
	testGame.Run();
	return 0;
}