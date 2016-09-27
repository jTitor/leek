#pragma once
#include "Datatypes.h"

namespace LeEK
{
	//standard axis values if they're undefined.
	const I32 AXIS_MAX = 0xFFFF;
	const I32 AXIS_MID = 0x8000;
	const I32 AXIS_MIN = 0x0000;

	//axis ID codes
	const U8 AXIS_X = 0x00;
	const U8 AXIS_Y = 0x01;
	const U8 AXIS_Z = 0x02;
	//rotational axii are a little unusual;
	//remember that Rz is for twist joysticks, it gets higher priority
	const U8 AXIS_RZ = 0x03;
	const U8 AXIS_RX = 0x04;
	const U8 AXIS_RY = 0x05;


	typedef U8 Key;

	//virtual keys!
	//Indicates an invalid key or unset keybinding.
	const Key KEY_INVALID = 0xFF;

	//letters
	const Key KEY_A = 0x41;
	const Key KEY_B = 0x42;
	const Key KEY_C = 0x43;
	const Key KEY_D = 0x44;
	const Key KEY_E = 0x45;
	const Key KEY_F = 0x46;
	const Key KEY_G = 0x47;
	const Key KEY_H = 0x48;
	const Key KEY_I = 0x49;
	const Key KEY_J = 0x4A;
	const Key KEY_K = 0x4B;
	const Key KEY_L = 0x4C;
	const Key KEY_M = 0x4D;
	const Key KEY_N = 0x4E;
	const Key KEY_O = 0x4F;
	const Key KEY_P = 0x50;
	const Key KEY_Q = 0x51;
	const Key KEY_R = 0x52;
	const Key KEY_S = 0x53;
	const Key KEY_T = 0x54;
	const Key KEY_U = 0x55;
	const Key KEY_V = 0x56;
	const Key KEY_W = 0x57;
	const Key KEY_X = 0x58;
	const Key KEY_Y = 0x59;
	const Key KEY_Z = 0x5A;
	//numbers
	const Key KEY_0 = 0x30;
	const Key KEY_1 = 0x31;
	const Key KEY_2 = 0x32;
	const Key KEY_3 = 0x33;
	const Key KEY_4 = 0x34;
	const Key KEY_5 = 0x35;
	const Key KEY_6 = 0x36;
	const Key KEY_7 = 0x37;
	const Key KEY_8 = 0x38;
	const Key KEY_9 = 0x39;
	//backspace
	const Key KEY_BKSP	= 0x08;
	const Key KEY_TAB	= 0x09;
	const Key KEY_ENTER	= 0x0D;
	const Key KEY_SPACE	= 0x20;
	//following keys are NOT same as ASCII codes.
	//numpad numbers
	const Key KEY_NPAD_0 = 0x5B;
	const Key KEY_NPAD_1 = 0x5C;
	const Key KEY_NPAD_2 = 0x5D;
	const Key KEY_NPAD_3 = 0x5E;
	const Key KEY_NPAD_4 = 0x5F;
	const Key KEY_NPAD_5 = 0x60;
	const Key KEY_NPAD_6 = 0x61;
	const Key KEY_NPAD_7 = 0x62;
	const Key KEY_NPAD_8 = 0x63;
	const Key KEY_NPAD_9 = 0x64;
	//modifier keys
	const Key KEY_LSHIFT = 0x00;
	const Key KEY_RSHIFT = 0x01;
	const Key KEY_LCTRL	= 0x02;
	const Key KEY_RCTRL	= 0x03;
	const Key KEY_LALT	= 0x04;
	const Key KEY_RALT	= 0x05;
	//lock keys
	const Key KEY_CAPLOCK = 0x3A;
	const Key KEY_NUMLOCK = 0x65;
	const Key KEY_SCRLOCK = 0x66;
	//arrow keys
	const Key KEY_UPARR	= 0x1C;
	const Key KEY_DNARR	= 0x1D;
	const Key KEY_LARR	= 0x1E;
	const Key KEY_RARR	= 0x1F;
	//misc characters.
	const Key KEY_GRAVE	= 0x06;
	const Key KEY_COMMA	= 0x07;
	const Key KEY_PERIOD	= 0x0A;//haven't been assigned right yet!
	const Key KEY_FSLASH	= 0x0B;
	const Key KEY_BKSLASH	= 0x11;
	//semicolon
	const Key KEY_SEMCOL	= 0x0C;
	//apostrophe
	const Key KEY_APOST		= 0x0E;
	//brackets "[]"
	const Key KEY_LBRACK	= 0x0F;
	const Key KEY_RBRACK	= 0x10;
	//misc characters.
	const Key KEY_EQUALS	= 0x12;
	const Key KEY_MINUS		= 0x13;
	//asterisk
	//fool, there's no asterisk key on a keyboard
	//const Key KEY_ASTRSK	= 0x14;
	const Key KEY_PAUSE		= 0x15;
	const Key KEY_ESCAPE	= 0x2D;
	//but there is that one key that does friggin' nothing good
	const Key KEY_APPS = 0x14;
	//print screen???
	const Key KEY_PRSCRN	= 0x16;
	const Key KEY_DELETE	= 0x17;
	const Key KEY_HOME	= 0x18;
	const Key KEY_END	= 0x19;
	const Key KEY_PGUP	= 0x1A;
	const Key KEY_PGDN	= 0x1B;
	//super keys - the start menu keys
	const Key KEY_LSUPR	= 0x2E;
	const Key KEY_RSUPR	= 0x2F;
	const Key KEY_NPAD_PLUS	= 0x3B;
	const Key KEY_NPAD_MINUS	= 0x3C;
	const Key KEY_NPAD_MULT	= 0x3D;
	const Key KEY_NPAD_DIV	= 0x3E;
	const Key KEY_NPAD_DOT	= 0x3F;
	const Key KEY_NPAD_ENTER	= 0x40;
	//function keys
	const Key KEY_F1	= 0x21;
	const Key KEY_F2	= 0x22;
	const Key KEY_F3	= 0x23;
	const Key KEY_F4	= 0x24;
	const Key KEY_F5	= 0x25;
	const Key KEY_F6	= 0x26;
	const Key KEY_F7	= 0x27;
	const Key KEY_F8	= 0x28;
	const Key KEY_F9	= 0x29;
	const Key KEY_F10	= 0x2A;
	const Key KEY_F11	= 0x2B;
	const Key KEY_F12	= 0x2C;

	//a name map for keys
	const char* GetKeyName(Key key);
}
