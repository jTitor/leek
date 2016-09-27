#include "ControllerConstants.h"

const char* keyNameMap[256];
bool mapReady = false;

using namespace LeEK;

void initNumbers()
{
	keyNameMap[KEY_0] = "0";
	keyNameMap[KEY_1] = "1";
	keyNameMap[KEY_2] = "2";
	keyNameMap[KEY_3] = "3";
	keyNameMap[KEY_4] = "4";
	keyNameMap[KEY_5] = "5";
	keyNameMap[KEY_6] = "6";
	keyNameMap[KEY_7] = "7";
	keyNameMap[KEY_8] = "8";
	keyNameMap[KEY_9] = "9";
	//also init the number pad
	keyNameMap[KEY_NPAD_0] = "Numpad 0";
	keyNameMap[KEY_NPAD_1] = "Numpad 1";
	keyNameMap[KEY_NPAD_2] = "Numpad 2";
	keyNameMap[KEY_NPAD_3] = "Numpad 3";
	keyNameMap[KEY_NPAD_4] = "Numpad 4";
	keyNameMap[KEY_NPAD_5] = "Numpad 5";
	keyNameMap[KEY_NPAD_6] = "Numpad 6";
	keyNameMap[KEY_NPAD_7] = "Numpad 7";
	keyNameMap[KEY_NPAD_8] = "Numpad 8";
	keyNameMap[KEY_NPAD_9] = "Numpad 9";
}

void initAlphabet()
{
	keyNameMap[KEY_A] = "A";
	keyNameMap[KEY_B] = "B";
	keyNameMap[KEY_C] = "C";
	keyNameMap[KEY_D] = "D";
	keyNameMap[KEY_E] = "E";
	keyNameMap[KEY_F] = "F";
	keyNameMap[KEY_G] = "G";
	keyNameMap[KEY_H] = "H";
	keyNameMap[KEY_I] = "I";
	keyNameMap[KEY_J] = "J";
	keyNameMap[KEY_K] = "K";
	keyNameMap[KEY_L] = "L";
	keyNameMap[KEY_M] = "M";
	keyNameMap[KEY_N] = "N";
	keyNameMap[KEY_O] = "O";
	keyNameMap[KEY_P] = "P";
	keyNameMap[KEY_Q] = "Q";
	keyNameMap[KEY_R] = "R";
	keyNameMap[KEY_S] = "S";
	keyNameMap[KEY_T] = "T";
	keyNameMap[KEY_U] = "U";
	keyNameMap[KEY_V] = "V";
	keyNameMap[KEY_W] = "W";
	keyNameMap[KEY_X] = "X";
	keyNameMap[KEY_Y] = "Y";
	keyNameMap[KEY_Z] = "Z";
}

void initCmdKeys()
{
	keyNameMap[KEY_BKSP] = "Backspace";
	keyNameMap[KEY_TAB] = "Tab";
	keyNameMap[KEY_ENTER] = "Enter";
	keyNameMap[KEY_SPACE] = "Space";
	keyNameMap[KEY_LSHIFT] = "Left Shift";
	keyNameMap[KEY_RSHIFT] = "Right Shift";
	keyNameMap[KEY_LCTRL] = "Left Ctrl";
	keyNameMap[KEY_RCTRL] = "Right Ctrl";
	keyNameMap[KEY_LALT] = "Left Alt";
	keyNameMap[KEY_RALT] = "Right Alt";
	keyNameMap[KEY_CAPLOCK] = "Caps Lock";
	keyNameMap[KEY_NUMLOCK] = "Num Lock";
	keyNameMap[KEY_SCRLOCK] = "Scroll Lock";
	keyNameMap[KEY_UPARR] = "Up Arrow";
	keyNameMap[KEY_DNARR] = "Down Arrow";
	keyNameMap[KEY_LARR] = "Left Arrow";
	keyNameMap[KEY_RARR] = "Right Arrow";
	keyNameMap[KEY_LSUPR] = "Left Super";
	keyNameMap[KEY_RSUPR] = "Right Super";
	keyNameMap[KEY_PAUSE] = "Pause";
	keyNameMap[KEY_ESCAPE] = "Escape";
	keyNameMap[KEY_PRSCRN] = "Print Screen";
	keyNameMap[KEY_DELETE] = "Delete";
	keyNameMap[KEY_HOME] = "Home";
	keyNameMap[KEY_END] = "End";
	keyNameMap[KEY_PGUP] = "Page Up";
	keyNameMap[KEY_PGDN] = "Page Down";
	keyNameMap[KEY_NPAD_ENTER] = "Numpad Enter";
	keyNameMap[KEY_APPS] = "Apps";

	keyNameMap[KEY_F1] = "F1";
	keyNameMap[KEY_F2] = "F2";
	keyNameMap[KEY_F3] = "F3";
	keyNameMap[KEY_F4] = "F4";
	keyNameMap[KEY_F5] = "F5";
	keyNameMap[KEY_F6] = "F6";
	keyNameMap[KEY_F7] = "F7";
	keyNameMap[KEY_F8] = "F8";
	keyNameMap[KEY_F9] = "F9";
	keyNameMap[KEY_F10] = "F10";
	keyNameMap[KEY_F11] = "F11";
	keyNameMap[KEY_F12] = "F12";
}

void initSymbols()
{
	keyNameMap[KEY_GRAVE] = "Grave";
	keyNameMap[KEY_COMMA] = ",";
	keyNameMap[KEY_PERIOD] = ".";
	keyNameMap[KEY_FSLASH] = "/";
	keyNameMap[KEY_BKSLASH] = "\\";
	keyNameMap[KEY_SEMCOL] = ";";
	keyNameMap[KEY_APOST] = "'";
	keyNameMap[KEY_LBRACK] = "[";
	keyNameMap[KEY_RBRACK] = "]";
	keyNameMap[KEY_EQUALS] = "=";
	keyNameMap[KEY_MINUS] = "-";
	keyNameMap[KEY_NPAD_PLUS] = "Numpad +";
	keyNameMap[KEY_NPAD_MINUS] = "Numpad -";
	keyNameMap[KEY_NPAD_MULT] = "Numpad *";
	keyNameMap[KEY_NPAD_DIV] = "Numpad /";
	keyNameMap[KEY_NPAD_DOT] = "Numpad .";
}

void initMisc()
{
	initCmdKeys();
	initSymbols();
}

void initNameMap()
{
	//set everything to a default string
	for(U32 i = 0; i < 256; ++i)
	{
		keyNameMap[i] = "UNUSED KEY";
	}

	initAlphabet();
	initNumbers();
	initMisc();
	mapReady = true;
}

const char* LeEK::GetKeyName(Key key)
{
	//simple map lookup
	if(!mapReady)
	{
		initNameMap();
	}
	return keyNameMap[key];
}
