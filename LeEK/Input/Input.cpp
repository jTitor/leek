#include "Input.h"
#include "Logging/Log.h"
#include "Strings/String.h"
#include "Platforms/IPlatform.h"

using namespace LeEK;

F32 Controller::GetAxisFiltered(U32 axis)
{
	L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
	ControllerAxis& ax = axisList[axis];
	F32 res = GetAxis(axis);
	if(ax.DeadZone > 0)
	{
		F32 deadZone = ax.DeadZone;
		//we want to normalize from the zone edge - the edge is zero but does not jump to its
		//unfiltered value past the edge
		if(Math::Abs(res) < deadZone)
		{
			return 0;
		}
		else
		{
			//this is 1D, so just get the sign
			res = (Math::Sign(res) * (res - deadZone)) / (1 - deadZone);
			return res;
		}
	}
	return res;
}

Vector2 Controller::GetAxisPairFiltered(U32 axisX, U32 axisY, ZoneType type)
{
	L_ASSERT(axisX < MAX_AXIS && axisY < MAX_AXIS && "Tried to access nonexistent axis!");
	F32 deadZoneX = axisList[axisX].DeadZone;
	F32 deadZoneY = axisList[axisY].DeadZone;
	F32 x = GetAxis(axisX);
	F32 y = GetAxis(axisY);
	Vector2 res(x, y);
	//there's a variety of zones
	switch(type)
	{
		//RADIUS creates a circular zone;
		//values are renormalized so you don't notice a "snap"
		//when you move out of the dead zone
		//note that this only uses the X zone!
	case RADIUS:
		{
			if(res.LengthSquared() < deadZoneX*deadZoneX || res.LengthSquared() == 0.0f)
			{
				return Vector2::Zero;
			}
			else
			{
				F32 axisLen = res.Length();
				res = ((res / axisLen)*(axisLen - deadZoneX))/(1 - deadZoneX);
				return res;
			}
			break;
		}
		//CROSS creates a cross shape; no renormalization.
	case CROSS:
		{
			if(Math::Abs(x) < deadZoneX)
			{
				res.SetX(0);
			}
			if(Math::Abs(y) < deadZoneY)
			{
				res.SetY(0);
			}
			return res;
			break;
		}
		//BOWTIE creates... well, a horizontal bowtie shape.
		//Y zone grows as you move further from X center
	case BOWTIE:
		{
			F32 scaledY = Math::Abs(x) * deadZoneY;
			if(Math::Abs(y) < scaledY)
			{
				res.SetY(0);
			}
			return res;
			break;
		}
	}
	return Vector2::Zero;
}

void Mouse::SwapStates()
{
	//since the arrays aren't dynamically set, we either need pointers to swap
	//or directly copy the memory
	//if directly copying, we're clearing the current state to NULL anyway, so we only need one copy
	memcpy_s(prevPos, sizeof(prevPos), currPos, sizeof(prevPos));
	//memset(currKeys, 0, sizeof(currKeys));
}

void Keyboard::SwapStates()
{
	//since the arrays aren't dynamically set, we either need pointers to swap
	//or directly copy the memory
	/*U64* swap = currPtr;
	currPtr = prevPtr;
	prevPtr = swap;
	memset(currPtr, 0, sizeof(U64)*4);*/
	//if directly copying, we're clearing the current state to NULL anyway, so we only need one copy
	/*for(U32 i = 0; i < 4; ++i)
	{
		if(currKeys[i] != 0 && !Math::IsPowerOfTwo(currKeys[i]))
		{
			U32 j = 0;
		}
	}*/
	memcpy_s(prevKeys, sizeof(prevKeys), currKeys, sizeof(prevKeys));
	//memset(currKeys, 0, sizeof(currKeys));
}

//InputManager
bool InputManager::Startup(IPlatform* plat)
{
	//procedure is:
	//	1. get raw input devices
	//	2. count the HIDs, mice, keyboards, init arrays as needed
	//	3. add the handles to the handle map
	//	4. notify platform we're interested in devices
	U32 numDevs = 0;
	numHID = 0;
	numMouse = 0;
	numKbd = 0;
	guiMousePos = Vector2::Zero;
	if(!plat)
	{
		LogW("No platform passed!");
		return false;
	}
	const InputDevInfo* devInfo = plat->FindInputDevList(&numDevs).Ptr();
	if(!devInfo)
	{
		LogW("Couldn't get input device info!");
		return false;
	}
	LogV("Listing device handles: ");
	for(U32 i = 0; i < numDevs; ++i)
	{
		LogV(String("Device ") + i + ": " + devInfo[i].PlatHandle);
	}
	//otherwise, start counting
	//we know from here what index the handle should have
	for(U32 i = 0; i < numDevs; ++i)
	{
		const InputDevInfo& dev = devInfo[i];
		L_ASSERT(ctrlHandleMap.count(dev.PlatHandle) <= 0 && "input device already added!");
		//note that devices can be invalidated by the OS if detailed data couldn't be found for them!
		if(dev.PlatHandle)
		{
			switch(dev.Type)
			{
			case InputDevInfo::HID:
				LogV(String("Handle ") + dev.PlatHandle + " is now HID index " + numHID);
				ctrlHandleMap[dev.PlatHandle] = numHID;
				numHID++;
				break;
			case InputDevInfo::Mouse:
				LogV(String("Handle ") + dev.PlatHandle + " is now mouse index " + numMouse);
				ctrlHandleMap[dev.PlatHandle] = 0;//numMouse;
				numMouse = 1;//++;
				break;
			//kinda weird; we want only one keyboard active
			//alias all the keyboards to one keyboard!
			case InputDevInfo::Keyboard:
				LogV(String("Handle ") + dev.PlatHandle + " is now keyboard index " + numKbd);
				ctrlHandleMap[dev.PlatHandle] = 0;//numKbd;
				numKbd = 1;//++;
				break;
			}
		}
	}
	LogV(String("Found ") + numDevs + " devices:");
	LogV(String("Assigned ") + numHID + " HIDs");
	LogV(String("Assigned ") + numMouse + " mice");
	LogV(String("Assigned ") + numKbd + " keyboards");
	//make default controllers, in case the user has no devices.
	U32 numHIDEntries = numHID == 0 ? 1 : numHID;
	U32 numMouseEntries = numMouse == 0 ? 1 : numMouse;
	U32 numKbdEntries = numKbd == 0 ? 1 : numKbd;

	//init the controller arrays
	ctrlrs = CustomArrayNew<Controller>(numHIDEntries, INPUT_ALLOC, "InputAlloc");
	mice = CustomArrayNew<Mouse>(numMouseEntries, INPUT_ALLOC, "InputAlloc");
	kbds = CustomArrayNew<Keyboard>(numKbdEntries, INPUT_ALLOC, "InputAlloc");
	//init each controller's axis ranges
	for(U32 i = 0; i < numDevs; ++i)
	{
		const InputDevInfo& dev = devInfo[i];
		if(dev.Type == InputDevInfo::HID)
		{
			Controller& ctrlr = ctrlrs[ctrlHandleMap[dev.PlatHandle]];
			ctrlr.Info.PlatHnd = dev.PlatHandle;
			ctrlr.Info.NumAxis = Math::Min(dev.NumAxis, (U8)Controller::MAX_AXIS);
			ctrlr.Info.NumBtns = dev.NumBtns;
			LogV(String("Initialized HID ") + ctrlHandleMap[dev.PlatHandle] + ":");
			LogV(String("\tNum Axis:\t") + ctrlr.Info.NumAxis);
			for(U32 j = 0; j < ctrlr.Info.NumAxis; ++j)
			{
				//set to the axis' midpoint so the control will display neutral position on polling
				ctrlr.SetAxisHalfRange(j, dev.Axii[j].HalfRange);
				ctrlr.SetAxisMidpoint(j, dev.Axii[j].Midpoint);
				ctrlr.SetAxisRawVal(j, (I32)dev.Axii[j].Midpoint);
				ctrlr.SetAxisDeadZone(j, 0.17f);
				LogV(String("\t\tAxis ") + j + ":");
				LogV(String("\t\t\tValue:\t") + ctrlr.GetAxis(j));
				LogV(String("\t\t\tHalf Range:\t") + ctrlr.GetAxisHalfRange(j));
				LogV(String("\t\t\tMidpoint:\t") + ctrlr.GetAxisMidpoint(j));
				LogV(String("\t\t\tDead Zone:\t") + ctrlr.GetAxisDeadZone(j));
			}
			LogV(String("\tNum Btns:\t") + ctrlr.Info.NumBtns);
		}
	}

	numHID = numHIDEntries;
	numMouse = numMouseEntries;
	numKbd = numKbdEntries;
	//we can delete the info now, it's redundant
	CustomArrayDelete(devInfo);
	return true;
}

void InputManager::Shutdown()
{
	//do any necessary releasing of input
	CustomArrayDelete(ctrlrs);
	CustomArrayDelete(mice);
	CustomArrayDelete(kbds);
}

void InputManager::Update()
{
	//just swap buttons on all controllers
	for(U32 i = 0; i < numHID; ++i)
	{
		ctrlrs[i].SwapStates();
	}

	for(U32 i = 0; i < numMouse; ++i)
	{
		mice[i].SwapStates();
		//also reset relative position as needed
		mice[i].SetMousePos(0,0);
	}

	for(U32 i = 0; i < numKbd; ++i)
	{
		kbds[i].SwapStates();
	}
}
