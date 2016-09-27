#include "Game.h"
#include "Stats/Profiling.h"
#include "Logging/Log.h"
#include "FileManagement/Filesystem.h"
#include "Time/DateTime.h"
#include "Platforms/Win32Platform.h"
#include "Platforms/LinuxPlatform.h"

using namespace LeEK;

const F32 PURGE_INTERVAL_MS = 1000.0f;
const char* DEFAULT_LOG_DIR = "/logs";
const char* DEFAULT_STAT_DIR = "/stats";

//TODO: make configurable
bool writeLogs = true;
bool writeStats = true;

Game::Game(void)
{
	plat = NULL;
	input = 0;
	graphics = 0;
	timeSincePurgeMs = 0;
	showWnd = true;
	running = false;
}


Game::~Game(void)
{
}

bool Game::initPlatform()
{
	if(!plat)
	{
#ifdef WIN32
		plat = CustomNew<Win32Platform>(PLAT_ALLOC, "PlatformAlloc");
#else
		plat = CustomNew<LinuxPlatform>(PLAT_ALLOC, "PlatformAlloc");
#endif
		if(!plat)
		{
			LogE("Could not alloc memory for platform manager!");
			plat = NULL;
			return false;
		}
		if(!plat->Startup())
		{
			//startup failed for some reason
			//platform is unsafe, do NOT return it.
			LogE("Could not init platform!");
			L_ASSERT(false && "Platform initialization failed!");
			CustomDelete(plat);
			plat = NULL;
			return false;
		}
	}
	//Link this into the filesystem.
	Filesystem::SetPlatform(plat);
	return true;
}

void Game::shutdownPlatform()
{
	if(plat)
	{
		plat->Shutdown();
		CustomDelete(plat);
		plat = NULL;
	}
}

void Game::initGfx(RendererType type, IPlatform* plat)
{
	if(!plat)
	{
		return;
	}

	//shut down the old instance
	if(graphics)
	{
		plat->ShutdownGraphicsWrapper(graphics);
	}

	//and init the new one
	IGraphicsWrapper* newInstance = plat->BuildGraphicsWrapper(type);
	//quit if initialization failed.
	if(!newInstance)
	{
		LogE("Couldn't init graphics wrapper!");
		return;
	}
	//was there an old instance?
	if(graphics)
	{
		//delete the old data
		LDelete(graphics.Ptr());
		//repoint the handle to the new instance!
		HandleMgr::MoveHandle(graphics, newInstance);
	}
	else
	{
		//instance built, set its handle
		graphics = HandleMgr::RegisterPtr(newInstance);
	}
}

bool Game::updateOS()
{
	PROFILE("OS Update");
	return plat->UpdateOS();
}

bool Game::Startup()
{
	//Create the platform-dependent module.
	if(!initPlatform())
	{
		LogE("Failed to init platform instance!");
		plat = NULL;
		return false;
	}
	Filesystem::SetPlatform(plat);

	//Create the log file, if necessary.
	if(writeLogs)
	{
		Path logPath = String(Filesystem::GetProgDir())+DEFAULT_LOG_DIR;
		if(!Filesystem::Exists(logPath))
		{
			if(!Filesystem::MakeDirectory(logPath))
			{
				//couldn't make directory, disable
				//logging
				Log::RAW("---Couldn't create log directory!---");
				Log::SetBufferEnabled(false);
			}
			else
			{
				Log::SetBufferEnabled(true);
				Log::OpenLogFile(DEFAULT_LOG_DIR);
			}
		}
		else
		{
			Log::SetBufferEnabled(true);
			Log::OpenLogFile(DEFAULT_LOG_DIR);
		}
	}
	//Same for the stats...
	if(writeStats)
	{
		Path statPath = String(Filesystem::GetProgDir())+DEFAULT_STAT_DIR;
		if(!Filesystem::Exists(statPath))
		{
			Filesystem::MakeDirectory(statPath);
		}
		stats.Startup();
		//stats.SetShouldPrint(false);
	}
	
	//next up is engine config - we'll make all these constants values that can be loaded.
	//load config here!
	input = HandleMgr::RegisterPtr(LNew(InputManager, AllocType::INPUT_ALLOC, "InputAlloc")());
	//under certain circumstances (namely tests), you might not want the engine to make a window.
	if(showWnd)
	{
		initGfx(OPEN_GL, plat);
		if(!graphics)
		{
			LogE("Failed to get graphics wrapper!");
			return false;
		}
		if(!plat->GetWindow(graphics, IPlatform::WINDOW, 1024, 768))
		{
			LogE("Failed to build window!");
			//no need to halt;
			//just report failure, Run() will call for shutdown
			return false;
		}
		bool inputInit = input->Startup(plat);
		L_ASSERT(inputInit && "Couldn't initialize input manager instance!");
		//link input manager w/ platform
		plat->InitInput(input);
	}
	else
	{
		bool inputInit = input->Startup(plat);
		L_ASSERT(inputInit && "Couldn't initialize input manager instance!");
	}
	running = true;
	return true;
}

void Game::Shutdown()
{
	//almost all of these calls are platform dependent,
	//so don't bother if the platform wasn't initialized
	if(plat)
	{
		//unhook the input manager
		plat->SetInputManager(0);
		//and shut it down, too
		input->Shutdown();
		if(showWnd)
		{
			plat->ShutdownGraphicsWrapper(graphics);
		}
		//Allocator::DumpAllocsCSV();
		//another place for engine config - can make the name different w/ settings
		if(writeStats)
		{
			String statPath = Filesystem::GetProgDir() + String(DEFAULT_STAT_DIR) + "/StatLog - " + DateTime::GetCurrDate() + ".csv";
			stats.DumpStatsCSV(statPath);
		}
		if(writeLogs)
		{
			Log::CloseLogFile();
		}
		shutdownPlatform();
	}
}

void Game::PreOS()
{
}

/*
void Game::Update(const GameTime& time)
{
	PROFILE("Update");
}


void Game::Draw(const GameTime& time)
{
	PROFILE("Render");
}
*/

void Game::fullUpdate(const GameTime& time)
{
	PROFILE("Update");
	Update(time);
	input->Update();
}

void Game::fullDraw(const GameTime& time)
{
	PROFILE("Render");
	{
		PROFILE("Render StartPrep");
		plat->BeginRender(graphics);
	}
	{
		PROFILE("Render Calls");
		Draw(time);
	}
	{
		PROFILE("Render EndPrep");
		plat->EndRender(graphics);
	}
}

void Game::updateAllocator(const GameTime& time)
{
	timeSincePurgeMs += time.ElapsedGameTime().ToMilliseconds();
	if(timeSincePurgeMs > PURGE_INTERVAL_MS)
	{
		Allocator::Purge();
		timeSincePurgeMs = 0;
	}
}

void Game::Run()
{
	if(!Startup())
	{
		Shutdown();
		return;
	}
	//bool running = true;
	while(running)
	{
		{
			PROFILE("Main Loop");
			//do stuff before OS here
			PreOS();
			if(updateOS())
			{
				break;
			}
			time.Tick();
			fullUpdate(time);
			fullDraw(time);
			//do cleanup as necessary
			{
				PROFILE("Cleanup");
				updateAllocator(time);
			}
		}
		stats.Update(time);
	}
	Shutdown();
}
