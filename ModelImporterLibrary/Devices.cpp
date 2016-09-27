#include "Devices.h"

using namespace LeEK;

IPlatform* Devices::gPlat = NULL;
GfxWrapperHandle Devices::gGfx = 0;
TypedHandle<InputManager> Devices::gInput = 0;

IPlatform* Devices::GetPlatform()
{
	if(!gPlat)
	{
	#ifdef WIN32
		gPlat = CustomNew<Win32Platform>(PLAT_ALLOC, "PlatformAlloc");
	#else
		gPlat = CustomNew<LinuxPlatform>(PLAT_ALLOC, "PlatformAlloc");
	#endif
		if(!gPlat)
		{
			LogE("Could not alloc memory for platform manager!");
			return NULL;
		}
		if(!gPlat->Startup())
		{
			//startup failed for some reason
			//platform is unsafe, do NOT return it.
			LogE("Could not init platform!");
			CustomDelete(gPlat);
			gPlat = NULL;
			return NULL;
		}
		//Link this into the filesystem.
		Filesystem::SetPlatform(gPlat);
	}
	return gPlat;
}

TypedHandle<InputManager> Devices::GetInput()
{
	if(!gInput)
	{
		gInput = HandleMgr::RegisterPtr(LNew(InputManager, AllocType::INPUT_ALLOC, "InputAlloc")());
		L_ASSERT(gInput && "Couldn't initialize input manager instance!");
		gInput->Startup(GetPlatform());
	}

	return gInput;
}

GfxWrapperHandle Devices::GetGfx()
{
	IPlatform* pPlat = GetPlatform();
	if(!gGfx)
	{
		if(!pPlat)
		{
			return 0;
		}

		/*
		//shut down the old instance
		if(instance)
		{
			plat->ShutdownGraphicsWrapper(instance);
		}
		*/

		//and init the new one
		IGraphicsWrapper* newInstance = pPlat->BuildGraphicsWrapper(OPEN_GL);
		//quit if initialization failed.
		if(!newInstance)
		{
			LogE("Couldn't init graphics wrapper!");
			return 0;
		}
		//was there an old instance?
		/*
		if(instance)
		{
			//delete the old data
			LDelete(instance.Ptr());
			//repoint the handle to the new instance!
			HandleMgr::MoveHandle(instance, newInstance);
		}
		else
		{
		}
		*/
		//instance built, set its handle
		gGfx = HandleMgr::RegisterPtr(newInstance);
	}
	return gGfx;
}