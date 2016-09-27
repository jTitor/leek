#pragma once
#include "Datatypes.h"
#include "EngineLogic/IEngineModule.h"
#include "Math/Vector2.h"
#include "DataStructures/STLContainers.h"
#include "Constants/ControllerConstants.h"
#include "Memory/Handle.h"
#include "StdAfx.h"
//#include "Platforms/IPlatform.h"

namespace LeEK
{
	/**
	*	This is a tough one.
	*	The input namespace handles everything regarding controllers;
	*	from here you can get the current list of controllers,
	*	get a specific controller, and a whole bunch of other junk.
	*/

	/**
	*	Individual controllers are represented by the Controller class.
	*	Controllers have:
	*		* a platform specific ID
	*		* a type:
	*			* Keyboard
	*			* Mouse
	*			* HID
	*		* an array of controls, represented either as bools (for digital buttons) or U16s (for analog controls).
	*		  Analog's stored as U16 rather than F32 because many joysticks store their values in [0 - 65535],
	*		  where 32768 (0x8000) represents the control's center.
	*		  You can get a [-1.0 - 1.0] value w/ a convienence function.
	*		* Generally, a controller will have state for this and the previous frame,
	*		  to allow tracking of button holds and presses.
	*/

	//currently, checking if buttons are pressed or released is hard,
	//since state is actively altered by the OS.
	//you can't ask for a snapshot yet, it seems.

	//Handles manipulation and display of button state.
	//Pass it a unsigned integral type,
	//and it uses the type as a bitfield.
	template<typename T>
	class ButtonSet
	{
	public:
		static const U32 MAX_SHIFT = sizeof(T)*8;
	protected:
		T currBtn;
		T prevBtn;
	public:
		ButtonSet()
		{
			currBtn = 0;
			prevBtn = 0;
		}
		static const U32 MaxNumBtns() { return MAX_SHIFT; };
		inline void SwapStates()
		{
			prevBtn = currBtn;
			currBtn = 0;
		}
		inline void SetButton(U8 btn)
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			currBtn |= (((U64)1) << btn);
		}
		inline void ClearButton(U8 btn)
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			currBtn &= ~(((U64)1) << btn);
		}
		inline void ClearAllButtons()
		{
			currBtn = 0;
		}
		//Get/Set dead zone functions?
		//down this frame
		inline bool ButtonDown(U8 btn) 
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			//need to specify a 64-bit shift
			return (((U64)currBtn) & (((U64)1) << btn)) != 0;
		}
		inline bool ButtonUp(U8 btn) { return !ButtonDown(btn); }
		//down both frames
		inline bool ButtonHeld(U8 btn)
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			U64 flag = ((U64)1) << btn;
			return (((U64)currBtn) & flag) && (((U64)prevBtn) & flag);
		}
		//up last frame, down this frame
		bool ButtonPressed(U8 btn)
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			U64 flag = ((U64)1) << btn;
			return (((U64)currBtn) & flag) && !(((U64)prevBtn) & flag);
		}
		//down last frame, up this frame
		bool ButtonReleased(U8 btn)
		{
			L_ASSERT(btn < MAX_SHIFT && "Attempted to access nonexistent button!");
			U64 flag = ((U64)1) << btn;
			return !(((U64)currBtn) & flag) && (((U64)prevBtn) & flag);
		}
	};

	struct ControllerInfo
	{
		U64 PlatHnd;
		U8 NumBtns, NumAxis;
	};

	enum ZoneType { RADIUS, CROSS, BOWTIE };

	class Controller : public ButtonSet<U64>
	{
	public:
		const static U32 MAX_AXIS = 16;
	private:
		//all axis are in the range [-1.0, 1.0].
		struct ControllerAxis
		{
			I32 Value;
			//the raw value on the control that maps to 0.0.
			//= (min raw value + half range)
			F32 Mid;
			//(max raw value - min raw value) / 2
			F32 HalfRange;
			F32 DeadZone;
		} axisList[MAX_AXIS];
	public:
		ControllerInfo Info;
		Controller()
		{
			for(U32 i = 0; i < MAX_AXIS; ++i)
			{
				axisList[i].Value = 0;
				axisList[i].Mid = 0;
				//to avoid divide by zero
				axisList[i].HalfRange = 1;
				axisList[i].DeadZone = 0;
			}
			Info.PlatHnd = 0;
			Info.NumAxis = 0;
			Info.NumBtns = 0;
		}
		inline F32 GetAxis(U32 axis)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			ControllerAxis& ax = axisList[axis];
			//need to clamp between this range; it's possible there's some rounding errors.
			F32 res = ((((F32)ax.Value) - ax.Mid)) / ax.HalfRange;
			return res;
		}
		inline I32 GetAxisRawVal(U32 axis)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			return axisList[axis].Value;
		}
		inline void SetAxisRawVal(U32 axis, I32 val)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			axisList[axis].Value = val;
		}
		inline F32 GetAxisMidpoint(U32 axis)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			return axisList[axis].Mid;
		}
		inline void SetAxisMidpoint(U32 axis, F32 mid)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			axisList[axis].Mid = mid;
		}
		inline F32 GetAxisHalfRange(U32 axis)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			return axisList[axis].HalfRange;
		}
		inline void SetAxisHalfRange(U32 axis, F32 halfRange)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			axisList[axis].HalfRange = halfRange;
		}
		inline F32 GetAxisDeadZone(U32 axis)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			return axisList[axis].DeadZone;
		}
		inline void SetAxisDeadZone(U32 axis, F32 deadZone)
		{
			L_ASSERT(axis < MAX_AXIS && "Tried to access nonexistent axis!");
			axisList[axis].DeadZone = deadZone;
		}
		F32 GetAxisFiltered(U32 axis);
		Vector2 GetAxisPairFiltered(U32 axisX, U32 axisY, ZoneType type);
		//Get/Set dead zone functions?
	};
	
	//very similar to a controller, but only has 2 axii.
	//RawInput can't handle more than 5 mouse buttons,
	//so this uses a U8 for button state.
	class Mouse : public ButtonSet<U8>
	{
	private:
		I32 currPos[2];
		I32 prevPos[2];

		I16 wheelDelta;
	public:
		Mouse()
		{
			currPos[0] = 0;
			currPos[1] = 0;

			wheelDelta = 0;
		}
		void SwapStates();
		inline Vector2 GetMousePos()
		{
			return Vector2((F32)currPos[0], (F32)currPos[1]);
		}
		inline Vector2 GetMouseDelta()
		{
			return Vector2(	(F32)(currPos[0] - prevPos[0]), 
							(F32)(currPos[1] - prevPos[1]));
		}
		inline I16 GetWheelDelta()
		{
			return wheelDelta;
		}
		inline void SetMousePos(I32 x, I32 y)
		{
			currPos[0] = x;
			currPos[1] = y;
		}
		inline void SetMousePos(Vector2 newPos)
		{
			SetMousePos((I32)newPos.X(), (I32)newPos.Y());
		}
		inline void SetWheelDelta(I16 delta)
		{
			wheelDelta = delta;
		}
	};

	/**
	*	Keyboard class.
	*	Standard procedure is to use keycodes defined in ControllerConstants.h.
	*	If you're calling SetKey, translate the keyboard's code/virtual key
	*	to these codes using the platform's getUniversalKeyCode() method.
	*/
	class Keyboard
	{
	private:
		//keyboard buttons can only ever be up or down, so we can use a bit field.
		//there are 256 possible keys; we can thus store them in an array of U64s.
		//a 1 indicates the key is pressed.
		U64 currKeys[4];
		U64 prevKeys[4];
		/*U64* currPtr;
		U64* prevPtr;*/
		inline Key getKeyMainIndex(Key chrCode)
		{
			//the keys are split amongst the 4 U64s,
			//so we can be sure a U8 won't be out of the bit range.
			//to figure out which U64 the code's flag will be in,
			//integer divide by 64 (the remainder will be the exact bit in the U64).
			//instead of dividing, we can just shift right by 6 bits,
			//since each shift is a division by 2.
			return chrCode >> 6;
		}
		inline Key getKeySubIndex(Key chrCode)
		{
			//hey hey, guess who was dumb enough to try bit fields?
			//from http://graphics.stanford.edu/~seander/bithacks.html#ModulusDivisionEasy.
			//idea is, pow2 - 1 sets all bits below the original 1.
			//dividing by that power of 2 would've shifted out those bits,
			//so to get the remainder, mask them out instead
			//this WILL be < 64.
			return chrCode & ((0x01 << 6)-1);
		}
		inline U64 getKeyFlag(Key chrCode)
		{
			return ((U64)1) << getKeySubIndex(chrCode);
		}
	public:
		Keyboard()
		{
			for(U32 i = 0; i < 4; ++i)
			{
				currKeys[i] = 0;
				prevKeys[i] = 0;
			}
		}
		void Startup()
		{
			/*currPtr = currKeys;
			prevPtr = prevKeys;*/
		}
		void SwapStates();
		//we will be given individual up/down messages for individual keys,
		//so we need corresponding functions
		inline void SetKey(Key key)
		{
			if(key == KEY_INVALID)
			{
				ClearKey(KEY_INVALID);
				return;
			}
			currKeys[getKeyMainIndex(key)] |= getKeyFlag(key);
		}
		inline void ClearKey(Key key)
		{
			currKeys[getKeyMainIndex(key)] &= ~getKeyFlag(key);
		}
		inline bool KeyDown(Key key) 
		{ 
			return (currKeys[getKeyMainIndex(key)] & getKeyFlag(key)) != 0;
		}
		inline bool KeyUp(Key key)
		{ 
			return !KeyDown(key);
		}
		bool KeyHeld(Key key)
		{
			Key index = getKeyMainIndex(key);
			U64 keyFlag = getKeyFlag(key);
			bool currDown = (currKeys[index] & keyFlag) != 0;
			bool prevDown = (prevKeys[index] & keyFlag) != 0;
			return currDown && prevDown;
		}
		bool KeyPressed(Key key)
		{
			Key index = getKeyMainIndex(key);
			U64 keyFlag = getKeyFlag(key);
			bool currDown = (currKeys[index] & keyFlag) != 0;
			bool prevDown = (prevKeys[index] & keyFlag) != 0;
			return currDown && !prevDown;
		}
		bool KeyReleased(Key key)
		{
			Key index = getKeyMainIndex(key);
			U64 keyFlag = getKeyFlag(key);
			bool currDown = (currKeys[index] & keyFlag) != 0;
			bool prevDown = (prevKeys[index] & keyFlag) != 0;
			return !currDown && prevDown;
		}
	};

	class IPlatform;

	class InputManager// : public IEngineModule
	{
	public:
	private:
		Controller* ctrlrs;
		Mouse* mice;
		Keyboard* kbds;
		U8 numHID;
		U8 numMouse;
		U8 numKbd;
		Vector2 guiMousePos;
		//converts handles to indices.
		//note that since each controller type has its own array,
		//there's going to be a bunch of handles with the same value;
		//however, this shouldn't be a problem, since you'll usually know beforehand what the handle's type is
		Map<U64, U8> ctrlHandleMap;
	public:

		InputManager()
		{
			numHID = 0;
			numMouse = 0;
			numKbd = 0;
		}
		~InputManager() {}
		bool Startup(IPlatform* plat);
		void Shutdown();
		//Since OS directly updates input,
		//you REALLY shouldn't call this...
		void Update();
		const U8 NumHID() { return numHID; }
		const U8 NumMouse() { return numMouse; }
		const U8 NumKbd() { return numKbd; }
		//functions regarding mouse's absolute position.
		//note that these give unnormalized coordinates.
		const Vector2& GetGUIMousePos() { return guiMousePos; }
		void SetGUIMousePos(const Vector2& newPos) { guiMousePos = newPos; }
		void SetGUIMousePos(F32 newX, F32 newY) { SetGUIMousePos(Vector2(newX, newY)); }
		//need to do error checking!
		U8 HandleToIndex(U64 hnd)
		{
			return ctrlHandleMap[hnd];
		}
		Controller& GetControllerByHandle(U64 hnd) 
		{ 
			return ctrlrs[HandleToIndex(hnd)];
		}
		Mouse& GetMouseByHandle(U64 hnd)
		{ 
			return mice[HandleToIndex(hnd)];
		}
		Keyboard& GetKeyboardByHandle(U64 hnd)
		{ 
			return kbds[HandleToIndex(hnd)];
		}
		Controller& GetController(U8 index) 
		{ 
			index = Math::Min(index, (U8)(numHID - 1));
			return ctrlrs[index];
		}
		Mouse& GetMouse(U8 index)
		{ 
			index = Math::Min(index, (U8)(numMouse - 1));
			return mice[index];
		}
		Keyboard& GetKeyboard(U8 index)
		{ 
			index = Math::Min(index, (U8)(numKbd - 1));
			return kbds[index];
		}
	};
}
