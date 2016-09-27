#ifdef WIN32
//#include "StdAfx.h"
#include <iostream>
#include "Win32Platform.h"
#include <windowsx.h>
#include <winreg.h>
#include "Win32Helpers.h"
#include "Libraries/GL_Loaders/WGL/wgl_core_4_3.h"
#include "Memory/Handle.h"
#include "GraphicsWrappers/NullGrpWrapper.h"
#include "DataStructures/STLContainers.h"
#include "DebugUtils/Assertions.h"
#include "Input/Input.h"
#include "Constants/ControllerConstants.h"

using namespace LeEK;

Win32Platform* Win32Platform::instance = NULL;
TCHAR pathBuffer[MAX_PATH];
String appPath = "";

#pragma region Input Fields
//virtual key map
const U32 KEY_MAP_LEN = 256;
U8 vKeyMap[KEY_MAP_LEN];

char* CtrlrTypeNames[3] = {"Mouse", "Keyboard", "HID"};
char* UsgPageNames[14] = {	"UNDEFINED",
							"Desktop",
							"Simulator",
							"VR",
							"Sport",
							"Game",
							"Generic",
							"Keyboard/pad",
							"LED",
							"Button",
							"Ordinal",
							"Telephony",
							"Consumer",
							"Digitizer" };
RAWINPUTDEVICE ridList[4];
//a little overkill, but you never know
const U32 RIBUF_LEN = 128;
char* rawInputBuf[RIBUF_LEN];
//should you check the next keyboard message for keypress data,
//even if it's not a key down event?
bool forceCheckKbdNextFrame = false;
#pragma endregion

#pragma region Function Pointers
//DLL handle for USB HID
HMODULE hidDLL = NULL;
//Function pointers for input funcs
typedef BOOLEAN		(__stdcall *FuncGetProdStr)(HANDLE hidHnd, PVOID buf, ULONG bufLen);
const char* StrGetProdStr = "HidD_GetProductString";
typedef NTSTATUS	(__stdcall *FuncGetCaps)(PHIDP_PREPARSED_DATA preData, PHIDP_CAPS caps);
const char* StrGetCaps = "HidP_GetCaps";
typedef NTSTATUS	(__stdcall *FuncGetBtnCaps)(HIDP_REPORT_TYPE repType, PHIDP_BUTTON_CAPS btnCaps,
											PUSHORT btnCapsLen, PHIDP_PREPARSED_DATA preData);
const char* StrGetBtnCaps = "HidP_GetButtonCaps";
typedef NTSTATUS	(__stdcall *FuncGetValCaps)(HIDP_REPORT_TYPE repType, PHIDP_VALUE_CAPS valCaps,
										   PUSHORT valCapsLen, PHIDP_PREPARSED_DATA preData);
const char* StrGetValCaps = "HidP_GetValueCaps";
typedef NTSTATUS	(__stdcall *FuncGetUsgs)(HIDP_REPORT_TYPE repType, USAGE usgPage,
										USHORT lnkColl, PUSAGE usgList, PULONG usgLen,
										PHIDP_PREPARSED_DATA preData, PCHAR report,
										ULONG repLen);
const char* StrGetUsgs = "HidP_GetUsages";
typedef NTSTATUS	(__stdcall *FuncGetUsgVal)(HIDP_REPORT_TYPE repType, USAGE usgPage,
										USHORT lnkColl, USAGE usg, PULONG usgVal,
										PHIDP_PREPARSED_DATA preData, PCHAR report,
										ULONG repLen);
const char* StrGetUsgVal = "HidP_GetUsageValue";
typedef NTSTATUS	(__stdcall *FuncGetUsgValArr)(HIDP_REPORT_TYPE repType, USAGE usgPage,
										USHORT lnkColl, USAGE usg, PCHAR usgVal,
										USHORT usgValByteLen, PHIDP_PREPARSED_DATA preData, 
										PCHAR report, ULONG repLen);
const char* StrGetUsgValArr = "HidP_GetUsageValueArray";

FuncGetProdStr getProductString;
FuncGetCaps getCaps;
FuncGetBtnCaps getButtonCaps;
FuncGetValCaps getValueCaps;
FuncGetUsgs getUsages;
FuncGetUsgVal getUsageValue;
FuncGetUsgValArr getUsageValueArray;

//audio
/*HMODULE oalDLL = NULL;
//function pointers defined by al.h
const char* StrALCGetStr = "alcGetString";
const char* StrALCExtPresent = "alcIsExtensionPresent";
const char* StrALExtPresent = "alIsExtensionPresent";
const char* StrALCOpenDev = "alcOpenDevice";
const char* StrALCCloseDev = "alcCloseDevice";
const char* StrALGetErr = "alGetError";

const char* StrALGenBufs = "alGenBuffers";
const char* StrALDelBufs = "alDeleteBuffers";
const char* StrAlBufData = "alBufferData";

const char* StrAlGenSrcs = "alGenSources";
const char* StrAlSrcI = "alSourcei";
const char* StrAlGetSrc3F = "alGetSource3f";
const char* StrAlSrcPlay = "alSourcePlay";

const char* StrAlGetListenerFV = "alGetListenerfv";
const char* StrAlListener3F = "alListener3f";

const char* StrAlCCreateContext = "alcCreateContext";
const char* StrAlCDestroyContext = "alcDestroyContext";
const char* StrAlCMakeContextCurr = "alcMakeContextCurrent";
const char* StrAlCGetCurrContext = "alcGetCurrentContext";
const char* StrAlCGetContextDev = "alcGetContextsDevice";*/
#pragma endregion

LPCTCH hidRegPath = L"SYSTEM\\CurrentControlSet\\Enum\\";

#pragma region Local Functions

char* getExePath()
{
	GetModuleFileName(NULL, pathBuffer, MAX_PATH);
#ifdef _UNICODE
	char convertedPath[MAX_PATH];
	//convert the path to something we can at least read as chars
	wcstombs_s(NULL, convertedPath, pathBuffer, _TRUNCATE);
	memcpy(pathBuffer, convertedPath, MAX_PATH*sizeof(char));
	return (char*)pathBuffer;
#endif
	return (char*)pathBuffer;
}

void initVKeyMap()
{
	//first, set each element to its index
	for(U32 i = 0; i < KEY_MAP_LEN; ++i)
	{
		vKeyMap[i] = i;
	}
	//since number and letter codes directly map to their index number
	//in Windows, we don't have to initialize them.
	//4 other codes also map directly:
	//	* VK_BACK
	//	* VK_TAB
	//	* VK_RETURN
	//	* VK_SPACE
	//init number pad
	vKeyMap[VK_NUMPAD0] = KEY_NPAD_0;
	vKeyMap[VK_NUMPAD1] = KEY_NPAD_1;
	vKeyMap[VK_NUMPAD2] = KEY_NPAD_2;
	vKeyMap[VK_NUMPAD3] = KEY_NPAD_3;
	vKeyMap[VK_NUMPAD4] = KEY_NPAD_4;
	vKeyMap[VK_NUMPAD5] = KEY_NPAD_5;
	vKeyMap[VK_NUMPAD6] = KEY_NPAD_6;
	vKeyMap[VK_NUMPAD7] = KEY_NPAD_7;
	vKeyMap[VK_NUMPAD8] = KEY_NPAD_8;
	vKeyMap[VK_NUMPAD9] = KEY_NPAD_9;
	//function keys
	vKeyMap[VK_F1] = KEY_F1;
	vKeyMap[VK_F2] = KEY_F2;
	vKeyMap[VK_F3] = KEY_F3;
	vKeyMap[VK_F4] = KEY_F4;
	vKeyMap[VK_F5] = KEY_F5;
	vKeyMap[VK_F6] = KEY_F6;
	vKeyMap[VK_F7] = KEY_F7;
	vKeyMap[VK_F8] = KEY_F8;
	vKeyMap[VK_F9] = KEY_F9;
	vKeyMap[VK_F10] = KEY_F10;
	vKeyMap[VK_F11] = KEY_F11;
	vKeyMap[VK_F12] = KEY_F12;
	//then command keys
	//windows has general keys, but also specific keys
	//for each side
	vKeyMap[VK_SHIFT] = KEY_LSHIFT;
	vKeyMap[VK_LSHIFT] = KEY_LSHIFT;
	vKeyMap[VK_RSHIFT] = KEY_RSHIFT;
	vKeyMap[VK_CONTROL] = KEY_LCTRL;
	vKeyMap[VK_LCONTROL] = KEY_LCTRL;
	vKeyMap[VK_RCONTROL] = KEY_RCTRL;
	vKeyMap[VK_MENU] = KEY_LALT;
	vKeyMap[VK_LMENU] = KEY_LALT;
	vKeyMap[VK_RMENU] = KEY_RALT;
	vKeyMap[VK_CAPITAL] = KEY_CAPLOCK;
	vKeyMap[VK_NUMLOCK] = KEY_NUMLOCK;
	vKeyMap[VK_SCROLL] = KEY_SCRLOCK;
	vKeyMap[VK_UP] = KEY_UPARR;
	vKeyMap[VK_DOWN] = KEY_DNARR;
	vKeyMap[VK_LEFT] = KEY_LARR;
	vKeyMap[VK_RIGHT] = KEY_RARR;
	vKeyMap[VK_LWIN] = KEY_LSUPR;
	vKeyMap[VK_RWIN] = KEY_RSUPR;
	vKeyMap[VK_LWIN] = KEY_LSUPR;
	vKeyMap[VK_RWIN] = KEY_RSUPR;
	vKeyMap[VK_APPS] = KEY_APPS;
	vKeyMap[VK_PAUSE] = KEY_PAUSE;
	vKeyMap[VK_ESCAPE] = KEY_ESCAPE;
	vKeyMap[VK_SNAPSHOT] = KEY_PRSCRN;
	vKeyMap[VK_DELETE] = KEY_DELETE;
	vKeyMap[VK_HOME] = KEY_HOME;
	vKeyMap[VK_END] = KEY_END;
	vKeyMap[VK_PRIOR] = KEY_PGUP;
	vKeyMap[VK_NEXT] = KEY_PGDN;
	//will not ordinarily be reached;
	//to trigger, check message data when you get
	//a VK_RETURN
	vKeyMap[VK_SEPARATOR] = KEY_NPAD_ENTER;
	//then symbols
	vKeyMap[VK_OEM_3] = KEY_GRAVE;
	vKeyMap[VK_OEM_COMMA] = KEY_COMMA;
	vKeyMap[VK_OEM_PERIOD] = KEY_PERIOD;
	vKeyMap[VK_OEM_2] = KEY_FSLASH;
	vKeyMap[VK_OEM_5] = KEY_BKSLASH;
	vKeyMap[VK_OEM_1] = KEY_SEMCOL;
	vKeyMap[VK_OEM_7] = KEY_APOST;
	vKeyMap[VK_OEM_4] = KEY_LBRACK;
	vKeyMap[VK_OEM_6] = KEY_RBRACK;
	vKeyMap[VK_OEM_PLUS] = KEY_EQUALS;
	vKeyMap[VK_OEM_MINUS] = KEY_MINUS;
	vKeyMap[VK_ADD] = KEY_NPAD_PLUS;
	vKeyMap[VK_SUBTRACT] = KEY_NPAD_MINUS;
	vKeyMap[VK_MULTIPLY] = KEY_NPAD_MULT;
	vKeyMap[VK_DIVIDE] = KEY_NPAD_DIV;
	vKeyMap[VK_DECIMAL] = KEY_NPAD_DOT;
}

bool loadFunction(HMODULE dll, const char* funcName, void* funcPtr)
{
	funcPtr = GetProcAddress(dll, funcName);
	if(!funcPtr)
	{
		LogE(String("Failed to load ") + funcName + "!");
		Win32Helpers::LogLastError();
		return false;
	}
	return true;
}

HWND HWNDFromWindowHnd(WindowHnd wHnd)
{
	return (HWND)wHnd;
}

//Hashes a USB HID usage to a single integer???
//Pretty sure this can be removed.
U32 hashUsage(U16 usgPage, U16 usg)
{
	return ((usgPage*17)*(usg-29)) % 16;
}
#pragma endregion

void streamFile(String name)
{
	//Need to know disk sector size;
	//buffers must be aligned by sector size
	//or async reads will hang.
	DWORD secSz = 0;
	{
		DWORD ignore1, ignore2, ignore3;
		//Replace hardcoded value with letter from path
		GetDiskFreeSpace(TEXT("C:\\"), &ignore1, &secSz, &ignore2, &ignore3);
	}

	//Open the file asynchronously.
	HANDLE f = CreateFile((LPCWSTR)name.c_str(),
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
	//Note failure to open.
	if(f == INVALID_HANDLE_VALUE)
	{
		LogE("Couldn't load file " + name + "!");
		return;
	}
	//Now try reads here...

	//We'll need an OVERLAPPED to monitor progress.
	OVERLAPPED ovrLp = {0};
	//Spawn an event we won't pay attention to.
	ovrLp.hEvent = CreateEvent( NULL,
								FALSE,	//auto-reset
								FALSE,
								NULL);	//no name

	//Make the required aligned buffer.
	char* readBuf = (char*)LAlignedMalloc(secSz, secSz, AllocType::PLAT_ALLOC, "TestStreamAlloc");

	//Now for the actual read.
	//In the actual case, we will probably care about that
	//bytesRead pointer.
	ReadFile(f, readBuf, secSz, NULL, &ovrLp );

	//ovrLp can now tell us if the read finished.
	DWORD bytesRead = 0;
	bool reading = true;
	while(reading)
	{
		//Need to check error code to see why a read's not complete.
		if(!GetOverlappedResult(f, &ovrLp, &bytesRead, FALSE))
		{
			switch(GetLastError())
			{
				//Still reading disk, settle down
			case ERROR_IO_INCOMPLETE:
				break;
				//At the end of the file.
			case ERROR_HANDLE_EOF:
				reading = false;
				break;
				//An actual error! Stop now!
			default:
				reading = false;
			}
		}
		//Otherwise, we're done, stop polling.
		else
		{
			reading = false;
		}
	}

	//Move by the number of bytes read and (presumably) continue reading.
	//Note that OVERLAPPED also has a OffsetHighBytes fieldfor 64-bit file positions;
	//for now we won't use it
	ovrLp.Offset += bytesRead;
}

Win32Platform::Win32Platform(void) : IPlatform()
{
	window = NULL;
	displayWidth = 0;
	displayHeight = 0;
	clientWidth = 0;
	clientHeight = 0;

	appName = L"";
	appInstance = NULL;
	devContext = NULL;
	msg = MSG();
	wndClassRegistered = false;

	renderContext = NULL;

	inputMgrHnd = 0;
	preParDataMap = Map<U64, PHIDP_PREPARSED_DATA>();
	numHID = 0;
	ctrlStats = NULL;
	btnStats = NULL;
	valStats = NULL;
}

Win32Platform::~Win32Platform(void)
{
}

void Win32Platform::GenRandomBytes(void* destBuf, U32 bufSizeBytes)
{
	Log::W("Using unimplemented function Win32Platform::GenRandomBytes!");
	//HCRYPTPROV* cryptProv;
	//CryptAcquireContext(
	//CryptGenRandom(bufSizeBytes, (BYTE*)destBuf);
}

const char* Win32Platform::FindFullProgPath()
{
	//const char* pathSep = __argv[0];
	char* rawPath = getExePath();
	//U32 len = strlen(rawPath);
	char* dirEndPos = strrchr(rawPath, '\\');
	dirEndPos[0] = 0;
	return rawPath;
}

const char* Win32Platform::GetProgDir()
{
	if(appPath.length() > 0)
	{
		return appPath.c_str();
	}
	//this returns the path to the .exe file, we want the directory
	String path = getExePath();
	U32 pos = path.find_last_of(FilesystemPathSeparator());
	appPath = path.substr(0, pos).c_str();
	appPath += '\\';
	return appPath.c_str();
}

bool Win32Platform::registerWindowClass(WNDCLASSEX* pWC)
{
	if(wndClassRegistered)
	{
		return true;
	}

	// Setup the windows class with default settings:
	pWC->style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //redraw if screen is changed (unlikely), and reserve a device context for this window so OpenGL works
	pWC->lpfnWndProc   = WindowProc;
	pWC->cbClsExtra    = 0;
	pWC->cbWndExtra    = 0;
	pWC->hInstance     = appInstance;
	pWC->hIcon         = LoadIcon(NULL, IDI_WINLOGO);
	pWC->hIconSm       = pWC->hIcon;
	pWC->hCursor       = LoadCursor(NULL, IDC_ARROW);
	pWC->hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	pWC->lpszMenuName  = NULL;
	pWC->lpszClassName = appName;
	pWC->cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(pWC);
	wndClassRegistered = true;
	return true;
}

bool Win32Platform::loadInputFuncs()
{
	hidDLL = LoadLibrary(L"HID.DLL");
	U32 err = GetLastError();

	//did we fail to load the DLL?
	if(!hidDLL)
	{
		LogE("Failed to load hid.dll!");
		//specify the error
		LPTSTR errStr = NULL;
		//Formatting from StackOverflow, Q#455434
		FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM 	// use system message tables to retrieve error text
						|FORMAT_MESSAGE_ALLOCATE_BUFFER // allocate buffer on local heap for error text
						|FORMAT_MESSAGE_IGNORE_INSERTS,  // Important! will fail otherwise, since we will not (and CANNOT) pass insertion parameters
						NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
						err,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&errStr,  // output 
						0, // minimum size for output buffer
						NULL);   // arguments - see note 
		std::wcout << L"Error: " << errStr << "\n";
		return false;
	}
	//otherwise, get the important functions
	//if ANY of them fail, report and unload.
	#pragma region Load HID.DLL Functions
	getProductString = (FuncGetProdStr)GetProcAddress(hidDLL, StrGetProdStr);
	if(!getProductString)
	{
		LogE(String("Failed to load ") + StrGetProdStr + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getCaps = (FuncGetCaps)GetProcAddress(hidDLL, StrGetCaps);
	if(!getCaps)
	{
		LogE(String("Failed to load ") + StrGetCaps + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getButtonCaps = (FuncGetBtnCaps)GetProcAddress(hidDLL, StrGetBtnCaps);
	if(!getButtonCaps)
	{
		LogE(String("Failed to load ") + StrGetBtnCaps + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getValueCaps = (FuncGetValCaps)GetProcAddress(hidDLL, StrGetValCaps);
	if(!getValueCaps)
	{
		LogE(String("Failed to load ") + StrGetValCaps + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getUsages = (FuncGetUsgs)GetProcAddress(hidDLL, StrGetUsgs);
	if(!getUsages)
	{
		LogE(String("Failed to load ") + StrGetUsgs + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getUsageValue = (FuncGetUsgVal)GetProcAddress(hidDLL, StrGetUsgVal);
	if(!getUsageValue)
	{
		LogE(String("Failed to load ") + StrGetUsgVal + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	getUsageValueArray = (FuncGetUsgValArr)GetProcAddress(hidDLL, StrGetUsgValArr);
	if(!getUsageValueArray)
	{
		LogE(String("Failed to load ") + StrGetUsgValArr + "!");
		FreeLibrary(hidDLL);
		return false;
	}
	#pragma endregion
	//we've loaded the functions, alright!
	return true;
}

bool Win32Platform::loadDLLs()
{
	//to be secure, remove CWD from search path to prevent DLL preloading
	//should only be done once in program!
	static U32 initCount = 0;
	initCount++;
	//L_ASSERT(initCount <= 1 && "Tried to init Win32Platform more than once!");
	LogV("Loading DLLs...");
	SetDllDirectory(L"");
	if(!loadInputFuncs())
	{
		LogE("Failed to load input functions!");
		return false;
	}
	return true;
}

bool Win32Platform::attachRenderer(GfxWrapperHandle gfx, HWND viewPort)
{
	//If there's no renderer, just show a blank screen
	if(!gfx)
	{
		return true;
	}

	switch(gfx->Type())
	{
	case OPEN_GL:
		{
			OGLGrpWrapper* castGfx = (OGLGrpWrapper*)gfx.Ptr();
			if(!oglInitRenderer(castGfx, viewPort,
								DEF_FARPLANE, DEF_NEARPLANE, DEF_VSYNC))
			{
				return false;
			}
			break;
		}
	case DIRECTX:
		//unimplemented; show a blank screen
		break;
	//otherwise, show a blank screen for now
	default:
		break;
	}
	return true;
}

bool Win32Platform::firstInitRenderer(IGraphicsWrapper* gfx)
{
	if(!gfx || functionsLoaded)
	{
		return true;
	}
	//might need a temporary window; if that's the case,
	//ensure we have the window class registered.
	if(!wndClassRegistered)
	{
		WNDCLASSEX wc;
		if(!registerWindowClass(&wc))
		{
			return false;
		}
	}

	switch(gfx->Type())
	{
	case OPEN_GL:
		{
			LogD("Entering OpenGL setup");
			//make a temp window for OpenGL setup
			window = CreateWindowEx(WS_EX_APPWINDOW, appName, appName, WS_POPUP,
			0, 0, 640, 480, NULL, NULL, appInstance, NULL);
			if(window == NULL)
			{
				Win32Helpers::LogLastError();
				return false;
			}
			//hide the window
			ShowWindow(window, SW_HIDE);
			//now run OpenGL setup!
			if(!oglLoadFunctions((OGLGrpWrapper*)gfx, window))
			{
				return false;
			}
			//now release the window
			DestroyWindow(window);
			window = NULL;
			break;
		}
	case DIRECTX:
		LogD("Entering DirectX setup");
		//TODO
		return false;
		break;
	//otherwise, show a blank screen for now
	default:
		LogW("Entering null API setup");
		break;
	}
	functionsLoaded = true;
	return true;
}

//handles messages from OS. Should return true if OS is telling the program to quit, false otherwise
bool Win32Platform::UpdateOS()
{
	// Handle windows messages.
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE))
	{
		//check if we're told to quit
		if(msg.message == WM_QUIT)
		{
			return true;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//update buffers
	return false;
}

bool Win32Platform::Startup()
{
	LogD("Initializing Win32 platform manager...");
	if(instance)
	{
		LogE("Win32 platform already initialized, aborting!");
		return false;
	}
	//initialize window and instance handles
	window = NULL;
	appInstance = NULL;
	inputMgrHnd = 0;

	//init input data
	numHID = 0;
	preParDataMap = Map<U64, PHIDP_PREPARSED_DATA>();
	ctrlStats = NULL;
	btnStats = NULL;
	valStats = NULL;

	//initialize display and window sizes
	displayWidth = GetSystemMetrics(SM_CXSCREEN);
	displayHeight = GetSystemMetrics(SM_CYSCREEN);
	clientWidth = 0;
	clientHeight = 0;
	isFullscreen = false;
	functionsLoaded = false;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	//the RI buffer, too.
	ZeroMemory(rawInputBuf, sizeof(rawInputBuf));

	// Get the instance of this application.
	appInstance = GetModuleHandle(NULL);

	// Give the application a name.
	appName = L"Engine";

	//load up any needed DLLs
	if(!loadDLLs())
	{
		LogE("Failed to load a DLL!");
		return false;
	}
	//fill out the vkey map
	initVKeyMap();
	instance = this;
	LogD("Initialized platform");
	return true;
}

void Win32Platform::Shutdown()
{
	LogD("Shutting down platform context");
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(isFullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	if(window)
	{
		DestroyWindow(window);
	}
	window = NULL;

	// Remove the application instance.
	if(appInstance)
	{
		UnregisterClass(appName, appInstance);
	}
	appInstance = NULL;

	//TODO: delete all the member fields...
	//in particular, iterate over the 2D arrays and maps.
	CustomArrayDelete(ctrlStats);
	if(numHID > 0)
	{
		for(U32 i = 0; i < numHID; ++i)
		{
			CustomArrayDelete(btnStats[i]);
			CustomArrayDelete(valStats[i]);
		}
		CustomArrayDelete(btnStats);
		CustomArrayDelete(valStats);
	}
	// Release the instance pointer.
	instance = NULL;

	//free any dlls.
	if(hidDLL)
	{
		FreeLibrary(hidDLL);
	}

	return;
}

WindowHnd Win32Platform::getWindow()
{
	return (WindowHnd)window;
}

#pragma region Input Methods
U8 Win32Platform::getUniversalKeyCode(U8 keyCode)
{
	return vKeyMap[keyCode];
}

bool Win32Platform::onUpdateInput(WPARAM wPar, LPARAM lPar)
{
	//this only updates if we have something to handle input in the first place
	if(inputMgrHnd)
	{
		#pragma region Init
		TypedHandle<InputManager> castHnd = inputMgrHnd;
		//data handle is the LPARAM
		//WPARAM indicates where window was when event happened, foreground or background
		U32 bufSz = 0;
		LONG err = 0;
		static const U32 MAX_BTNS = 64;
		static USAGE usageList[MAX_BTNS];
		static U32 axisVal = 0;

		//get buffer size
		err = GetRawInputData((HRAWINPUT)lPar, RID_INPUT, NULL, &bufSz, sizeof(RAWINPUTHEADER));
		if(err == -1)
		{
			//we couldn't get the buffer size!
			return false;
		}
		bufSz = RIBUF_LEN;
		//get the actual data
		err = GetRawInputData((HRAWINPUT)lPar, RID_INPUT, rawInputBuf, &bufSz, sizeof(RAWINPUTHEADER));
		RAWINPUT* riBuf = (RAWINPUT*)(void*)rawInputBuf;
		if(err == -1)
		{
			//we've got a unknown problem
			return false;
		}
		#pragma endregion
		//now data should be filled
		//do whatever here
		//TODO: We could maybe optimize this by using the type to convert to known type enums in InputManager
		//and restructure InputManager such that all controllers are stored in a 2D array - indexed first by type,
		//then by individual number.
		//By doing so, we could eliminate switches in calls to InputManager.
		switch(riBuf->header.dwType)
		{
		case RIM_TYPEHID:
			{
				#pragma region Update HID
				//get controller
				Controller& ctrlr = castHnd->GetControllerByHandle((U64)riBuf->header.hDevice);
				//reinterpret as HID. You'll need the preparsed data.
				RAWHID& hidDat = riBuf->data.hid;
				//get the pointer to that data while we're at it
				//better not reach this!
				if(preParDataMap.count((U64)riBuf->header.hDevice) < 1)
				{
					break;
				}
				PHIDP_PREPARSED_DATA preParDat = preParDataMap[(U64)riBuf->header.hDevice];
				//first, get the buttons
				//only get page 0 because we're dumb!
				//TODO: test for if buttons AREN'T a range
				U8 index = castHnd->HandleToIndex((U64)riBuf->header.hDevice);
				HIDP_BUTTON_CAPS* btnCaps = btnStats[index];
				U32 btnRange = btnCaps[0].Range.UsageMax - btnCaps[0].Range.UsageMin;
				//clear button state now
				getUsages(	HidP_Input, btnCaps[0].UsagePage, 0, usageList,
							(PULONG)&btnRange, preParDat, (PCHAR)hidDat.bRawData, hidDat.dwSizeHid);
				ctrlr.ClearAllButtons();
				for(U32 i = 0; i < btnRange; ++i)
				{
					ctrlr.SetButton(usageList[i] - btnStats[index][0].Range.UsageMin);
				}
				//next, get the axis
				HIDP_VALUE_CAPS* valCaps = valStats[index];
				axisVal = 0;
				for(U32 i = 0; i < ctrlr.Info.NumAxis; ++i)
				{
					//TODO: test for if axis is a range
					getUsageValue(	HidP_Input, valCaps[i].UsagePage, 0, valCaps[i].NotRange.Usage,
									(PULONG)&axisVal, preParDat, (PCHAR)hidDat.bRawData, hidDat.dwSizeHid);
					//now set axis value
					ctrlr.SetAxisRawVal(i, axisVal);
				}
				#pragma endregion
				break;
			}
			//otherwise, we can interpret this right here!
		case RIM_TYPEMOUSE:
			{
				if(shouldLockMouse)
				{
					//reset the mouse position
					POINT pt = POINT();
					pt.x = clientWidth / 2;
					pt.y = clientHeight / 2;
					ClientToScreen(window, &pt);
					SetCursorPos(pt.x, pt.y);
				}
				#pragma region Update Mouse
				//get the needed mouse
				Mouse& mouse = castHnd->GetMouseByHandle((U64)riBuf->header.hDevice);
				//also setup a shortcut for the data
				const RAWMOUSE& mouseDat = riBuf->data.mouse;
				//set mouse data
				mouse.SetMousePos(mouseDat.lLastX, mouseDat.lLastY);
				//set mouse wheel as needed
				if((mouseDat.usButtonFlags & RI_MOUSE_WHEEL) != 0)
				{
					mouse.SetWheelDelta(mouseDat.usButtonData);
				}
				U16 flags = mouseDat.usButtonFlags;

				//now also set mouse buttons
				//buttons are WEIRD under RI.
				//there's flags indicating buttons being pressed and released,
				//and buttons hold multiple positions
				//(M1 has 01b for down and 10b for up, for instance).
				//since buttons are cleared on update, however, we can just check for button down
				if(flags & RI_MOUSE_BUTTON_1_DOWN)
				{
					mouse.SetButton(0);
				}
				if(flags & RI_MOUSE_BUTTON_2_DOWN)
				{
					mouse.SetButton(1);
				}
				if(flags & RI_MOUSE_BUTTON_3_DOWN)
				{
					mouse.SetButton(2);
				}
				if(flags & RI_MOUSE_BUTTON_4_DOWN)
				{
					mouse.SetButton(3);
				}
				if(flags & RI_MOUSE_BUTTON_5_DOWN)
				{
					mouse.SetButton(4);
				}
				//also check if buttons have been released
				if(flags & RI_MOUSE_BUTTON_1_UP)
				{
					mouse.ClearButton(0);
				}
				if(flags & RI_MOUSE_BUTTON_2_UP)
				{
					mouse.ClearButton(1);
				}
				if(flags & RI_MOUSE_BUTTON_3_UP)
				{
					mouse.ClearButton(2);
				}
				if(flags & RI_MOUSE_BUTTON_4_UP)
				{
					mouse.ClearButton(3);
				}
				if(flags & RI_MOUSE_BUTTON_5_UP)
				{
					mouse.ClearButton(4);
				}
				#pragma endregion
				break;
			}
		case RIM_TYPEKEYBOARD:
			{
				Keyboard& kbd = castHnd->GetKeyboardByHandle((U64)riBuf->header.hDevice);
				const RAWKEYBOARD& kbdDat = riBuf->data.keyboard;
				//each message carries ONE VKey.
				//we need to convert to universal key code
				//will need some fixing, check Molecular Musings post in docs.
				USHORT res = kbdDat.Flags & RI_KEY_BREAK;
 				if((kbdDat.Flags & RI_KEY_BREAK) != RI_KEY_BREAK)
				{
					kbd.SetKey(getUniversalKeyCode((U8)kbdDat.VKey));
				}
				else if((kbdDat.Flags & RI_KEY_BREAK) == RI_KEY_BREAK)
				{
					//key must've been released, clear it
					kbd.ClearKey(getUniversalKeyCode((U8)kbdDat.VKey));
				}
				break;
			}
		}
		//castHnd->Update();
		return true;
	}
	return false;
}

void Win32Platform::onUpdateGUI(LPARAM lPar)
{
	if(inputMgrHnd)
	{
		TypedHandle<InputManager> castHnd = inputMgrHnd;
		castHnd->SetGUIMousePos((F32)GET_X_LPARAM(lPar), (F32)GET_Y_LPARAM(lPar));
	}
}

void Win32Platform::SetInputManager(Handle inMgrHnd)
{
	inputMgrHnd = inMgrHnd;
}

TypedHandle<InputDevInfo> Win32Platform::FindInputDevList(U32* outDevs)
{
	LogV("Finding input devices...");
	U32 numDevs = 0;
	//Get the number of controllers
	GetRawInputDeviceList(NULL, &numDevs, sizeof(RAWINPUTDEVICELIST));
	//notify output var of the device count
	LogD(String("Found ") + numDevs + " controllers");
	*outDevs = 0;//numDevs;
	//if there's no applicable devices, give up
	if(numDevs < 1)
	{
		LogW("No raw input devices attached!");
		return 0;
	}
	//now create the device info
	RAWINPUTDEVICELIST* devices = CustomArrayNew<RAWINPUTDEVICELIST>(numDevs, PLAT_ALLOC, "RawDevListAlloc");
	memset(devices, 0, numDevs*sizeof(RAWINPUTDEVICELIST));
	InputDevInfo* devInfoList = CustomArrayNew<InputDevInfo>(numDevs, PLAT_ALLOC, "RawDevListAlloc");
	//register the info list with handle system
	TypedHandle<InputDevInfo> infoHnd = HandleMgr::RegisterPtr(devInfoList);
	//if we couldn't get a handle, also give up
	if(!infoHnd.GetHandle())
	{
		LogW("Couldn't get handle for device info!");
		return 0;
	}
	//get actual device data
	GetRawInputDeviceList(devices, &numDevs, sizeof(RAWINPUTDEVICELIST));
	//init capability information array
	HIDP_CAPS* ctrlStatsTemp = CustomArrayNew<HIDP_CAPS>(numDevs, PLAT_ALLOC, "RawDevListAlloc");
	memset(ctrlStatsTemp, 0, numDevs*sizeof(HIDP_CAPS));
	//Make the HID capability lists.
	//Note that these are both arrays of arrays;
	//each device can have more than one capability.
	//In practice, game controllers & joysticks only have multiple value caps,
	//and one usg. range for buttons.
	HIDP_BUTTON_CAPS** btnStatsTemp = CustomArrayNew<HIDP_BUTTON_CAPS*>(numDevs, PLAT_ALLOC, "RawDevListAlloc");
	memset(btnStatsTemp, 0, numDevs*sizeof(HIDP_BUTTON_CAPS*));
	HIDP_VALUE_CAPS** valStatsTemp = CustomArrayNew<HIDP_VALUE_CAPS*>(numDevs, PLAT_ALLOC, "RawDevListAlloc");
	memset(valStatsTemp, 0, numDevs*sizeof(HIDP_VALUE_CAPS*));
	U32 bufLen = 0;

	//fill RI device list
	for(U32 i = 0; i < numDevs; ++i)
	{
		LONG err = 0;
		InputDevInfo& info = devInfoList[i];
		//set handle
		//not ultra safe, but what're the chances we'll be using 128-bit processors anytime soon?
		info.PlatHandle = (size_t)devices[i].hDevice;
		LogV(String("Setting device ") + i + " handle to " + info.PlatHandle);
		DWORD type = devices[i].dwType;
		//set type
		switch(devices[i].dwType)
		{
		case RIM_TYPEHID:
			{
				//note that this is an actual device
				LogV(String("\tFound HID ") + info.PlatHandle);
				*outDevs += 1;
				#pragma region Init HID Data
				info.Type = InputDevInfo::HID;
				//fill in additional data
				//also need to get the preparsed data
				bufLen = 0;
				err = GetRawInputDeviceInfo(devices[i].hDevice, RIDI_PREPARSEDDATA, NULL, &bufLen);
				//if we somehow couldn't get the preparsed data size,
				//there's no getting more information here;
				//go to the next controller.
				if(err < -1)
				{
					Log::W(String("Could not get allocation size for device ") + i + "!");
					Win32Helpers::LogLastError();
					//invalidate this device
					info.PlatHandle = 0;
					info.NumAxis = 0;
					info.NumBtns = 0;
					break;
				}
				//save this data to the map
				preParDataMap[info.PlatHandle] = (PHIDP_PREPARSED_DATA)Allocator::_CustomMalloc(bufLen, PLAT_ALLOC, "RawDevListAlloc", __FILE__, __LINE__);
				L_ASSERT(preParDataMap[info.PlatHandle] != NULL && "Ran out of memory for preparsed HID data!");
				//load preparsed data
				err = GetRawInputDeviceInfo((HANDLE)info.PlatHandle, RIDI_PREPARSEDDATA, preParDataMap[info.PlatHandle], &bufLen);
				//if we couldn't get the prepar. data...
				if(err < -1)
				{
					Log::W(String("Could not get dev info for device ") + i + "!");
					Win32Helpers::LogLastError();
					//invalidate this device
					info.PlatHandle = 0;
					info.NumAxis = 0;
					info.NumBtns = 0;
					break;
				}
				LogV(String("\tPulled device info for ") + info.PlatHandle);
				//get detailed data now
				getCaps(preParDataMap[info.PlatHandle], &ctrlStatsTemp[i]);
				//set cap data in the info struct
				U16 numBtnCaps = ctrlStatsTemp[i].NumberInputButtonCaps;
				info.NumBtns = 0;
				U16 numValCaps = ctrlStatsTemp[i].NumberInputValueCaps;
				info.NumAxis = 0;
				//make cap arrays for this device
				btnStatsTemp[i] = CustomArrayNew<HIDP_BUTTON_CAPS>(numBtnCaps, PLAT_ALLOC, "RawDevListAlloc");
				valStatsTemp[i] = CustomArrayNew<HIDP_VALUE_CAPS>(numValCaps, PLAT_ALLOC, "RawDevListAlloc");
				//fill cap arrays
				LogV(String("\tLoading button info for ") + info.PlatHandle + "...");
				getButtonCaps(HidP_Input, btnStatsTemp[i], &numBtnCaps, preParDataMap[info.PlatHandle]);
				LogV(String("\tLoaded button info for ") + info.PlatHandle + ", parsing data");
				for(U16 j = 0; j < numBtnCaps; ++j)
				{
					HIDP_BUTTON_CAPS& btnCap = btnStatsTemp[i][j];
					if(btnCap.IsRange)
					{
						info.NumBtns += (btnCap.Range.UsageMax - btnCap.Range.UsageMin);
					}
					else
					{
						info.NumBtns++;
					}
				}
				LogV(String("\tParsed button info for ") + info.PlatHandle);
				LogV(String("\tLoading axis info for ") + info.PlatHandle + "...");
				getValueCaps(HidP_Input, valStatsTemp[i], &numValCaps, preParDataMap[info.PlatHandle]);
				//cap the number of axii to the info struct's maximum axii,
				//to avoid corrupting the structure
				numValCaps = Math::Min(numValCaps, (U16)Controller::MAX_AXIS);
				LogV(String("\tLoaded axis info for ") + info.PlatHandle + ", parsing data");
				//setup axis data
				for(U16 j = 0; j < numValCaps; ++j)
				{
					LogV(String("Parsing axis ") + j + " for device " + info.PlatHandle);
					HIDP_VALUE_CAPS& valCap = valStatsTemp[i][j];
					if(valCap.IsRange)
					{
						info.NumAxis += (valCap.Range.UsageMax - valCap.Range.UsageMin);
					}
					else
					{
						info.NumAxis++;
					}

					//hat switches are special; don't do any scaling to their values
					//(midpoint = 0 and halfRange = 1)
					if(valCap.NotRange.Usage == 0x39)
					{
						info.Axii[j].HalfRange = 1;
						info.Axii[j].Midpoint = 0;
					}
					else
					{
						//if there's no specified max, we'll need to make an assumption
						//some controllers (like the 360) use REALLY high max values (0xffffffff)
						//that would be considered negative values if cast to signed values.
						//this is invalid too!
						if(valCap.LogicalMax <= 0)
						{
							info.Axii[j].HalfRange = ((F32)(AXIS_MAX - AXIS_MIN)) / 2;
							info.Axii[j].Midpoint = AXIS_MID;
						}
						else
						{
							F32 max = (F32)valCap.LogicalMax;
							F32 min = (F32)valCap.LogicalMin;
						
							info.Axii[j].HalfRange = (max - min) / 2;
							//if the half range is 0, this is going to be a huge pain!!!
							if(info.Axii[j].HalfRange == 0.0f)
							{
								info.Axii[j].HalfRange = ((F32)(AXIS_MAX - AXIS_MIN)) / 2;
								info.Axii[j].Midpoint = AXIS_MID;
							}
							else
							{
								info.Axii[j].Midpoint = (min + info.Axii[j].HalfRange);
							}
						}
					}
				}
				LogV(String("\tParsed axis info for ") + info.PlatHandle);
				#pragma endregion
				break;
			}
			//Mice and keyboards are much simpler, note the type and quit gracefully.
			//All the input manager needs to know is the device handle, and we've taken care of that.
		case RIM_TYPEMOUSE:
			{
				LogV(String("\tFound mouse ") + info.PlatHandle);
				//note that this is an actual device
				*outDevs += 1;
				info.Type = InputDevInfo::Mouse;
				info.NumAxis = 0;
				info.NumBtns = 0;
				break;
			}
		case RIM_TYPEKEYBOARD:
			{
				LogV(String("\tFound keyboard ") + info.PlatHandle);
				//note that this is an actual device
				*outDevs += 1;
				info.Type = InputDevInfo::Keyboard;
				info.NumAxis = 0;
				info.NumBtns = 0;
				break;
			}
		default:
			{
				LogV(String("Found invalid device ") + info.PlatHandle + "!");
				info.PlatHandle = 0;
				info.NumAxis = 0;
				info.NumBtns = 0;
				break;
			}
		}
		LogV(String("\tDevice ID for device ") + i + ": " + info.PlatHandle);
	}

	//and cleanup the allocations
	CustomArrayDelete(devices);
	for(U32 i = 0; i < numDevs; ++i)
	{
		CustomArrayDelete(btnStatsTemp[i]);
		CustomArrayDelete(valStatsTemp[i]);
	}
	CustomArrayDelete(btnStatsTemp);
	CustomArrayDelete(valStatsTemp);
	CustomArrayDelete(ctrlStatsTemp);

	LogV("All input devices found, listing output: ");
	for(unsigned int i = 0; i < *outDevs; ++i)
	{
		LogV(String("Device ") + i + ": " + infoHnd.Ptr()[i].PlatHandle);
	}
	return infoHnd;
}

void Win32Platform::buildHIDDetailList()
{
	if(inputMgrHnd)
	{
		LogV("Building HID details...");
		TypedHandle<InputManager> inputMgr = inputMgrHnd;
		numHID = inputMgr->NumHID();
		LogV(String("Found ") + numHID + " HIDs");
		//note that this should only be done when the cap data's invalid
		L_ASSERT(!ctrlStats && !btnStats & !valStats && "Tried to build input device list when list has already been built!");
		//only HIDs need this data, all else can be directly read via API calls
		ctrlStats = LArrayNew(HIDP_CAPS, numHID, INPUT_ALLOC, "RawDevListAlloc");
		btnStats = LArrayNew(HIDP_BUTTON_CAPS*, numHID, INPUT_ALLOC, "RawDevListAlloc");
		memset(btnStats, 0, numHID*sizeof(HIDP_BUTTON_CAPS*));
		valStats = LArrayNew(HIDP_VALUE_CAPS*, numHID, INPUT_ALLOC, "RawDevListAlloc");
		memset(valStats, 0, numHID*sizeof(HIDP_VALUE_CAPS*));
		//retraverse device list!
		for(U8 i = 0; i < numHID; ++i)
		{
			//check for index's existence in manager
			Controller& ctrlr = inputMgr->GetController(i);
			auto preParCtrlr = preParDataMap[ctrlr.Info.PlatHnd];
			LogV(String("Controller ") + i + " found with handle " + HexStrFromVal(ctrlr.Info.PlatHnd));
			//get detailed data now
			getCaps(preParCtrlr, &ctrlStats[i]);
			//make cap arrays for this device
			U16 numBtnCaps = ctrlStats[i].NumberInputButtonCaps;
			U16 numValCaps = ctrlStats[i].NumberInputValueCaps;
			btnStats[i] = LArrayNew(HIDP_BUTTON_CAPS, numBtnCaps, PLAT_ALLOC, "RawDevListAlloc");
			valStats[i] = LArrayNew(HIDP_VALUE_CAPS, numValCaps, PLAT_ALLOC, "RawDevListAlloc");
			//fill cap arrays
			getButtonCaps(HidP_Input, btnStats[i], &numBtnCaps, preParCtrlr);
			getValueCaps(HidP_Input, valStats[i], &numValCaps, preParCtrlr);
		}
		LogV("Built HID details");
	}
	else
	{
		LogW("No input manager attached, couldn't build HID details!");
	}
}

void Win32Platform::registerAllInputDevices(HWND targetWindow)
{
	//want to register:
	//joysticks
	//gamepads
	//keyboard
	//mouse
	
	//RAWINPUTDEVICE& joyRID = ridList[0];
	ridList[0].usUsagePage = 1;	
	ridList[0].usUsage = 4;			//joystick
	ridList[0].dwFlags = 0;			//no special options
	ridList[0].hwndTarget = targetWindow;	//window is foreground handle

	ridList[1].usUsagePage = 1;	
	ridList[1].usUsage = 5;			//gamepad
	ridList[1].dwFlags = 0;			//no special options
	ridList[1].hwndTarget = targetWindow;

	ridList[2].usUsagePage = 1;	
	ridList[2].usUsage = 6;					//keyboard
	//disable legacy messages;
	//we don't want the start menu buttons actually bringing up the menu, for example
	ridList[2].dwFlags = RIDEV_NOLEGACY;
	ridList[2].hwndTarget = targetWindow;

	ridList[3].usUsagePage = 0x01;
	ridList[3].usUsage = 0x02;					//mouse
	//Do NOT disable legacy messages for the mouse.
	//We don't intercept those messages, but the OS does.
	//In targetWindowed mode, that would make the targetWindow not respond to any commands.
	//Plus, it's also useful for GUI work!
	ridList[3].dwFlags = 0;
	ridList[3].hwndTarget = targetWindow;

	if(!RegisterRawInputDevices(ridList, 4, sizeof(RAWINPUTDEVICE)))
	{
		LogD("Couldn't register raw input devices!");
		Win32Helpers::LogLastError();
	}
	LogD("Registered raw input devices");
	U32 numDevs = 0;
	GetRegisteredRawInputDevices(NULL, &numDevs, sizeof(RAWINPUTDEVICE));
	LogD(String("OS recognizes ") + numDevs + " registered categories");
}

void Win32Platform::InitInput(WindowHnd targetWindow, TypedHandle<InputManager> inputMgr)
{
	if(!window)
	{
		LogW("InitInput: target window is invalid, aborting!");
		return;
	}

	if(!inputMgr)
	{
		LogW("InitInput: couldn't get input manager instance, aborting!");
		return;
	}

	SetInputManager(inputMgr);
	buildHIDDetailList();
	//and tell platform we're ready for input
	registerAllInputDevices((HWND)targetWindow);
}
#pragma endregion

#pragma region Graphics Methods
bool Win32Platform::GetWindow(GfxWrapperHandle grpWrapper, WindowType type, U32 width, U32 height)
{
	//first, build a window
	WNDCLASSEX wc;
	DEVMODE dmDisplaySettings;
	U32 posX, posY;

	if(!registerWindowClass(&wc))
	{
		return false;
	}
	//if we have a renderer, call a renderer specific startup function
	if(!firstInitRenderer(grpWrapper.Ptr()))
	{
		return false;
	}

	//Specify window details, necessary to calculate proper window size.
	DWORD winStyle = isFullscreen ? WS_POPUP : (WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME));
	U32 windowWidth = 0;
	U32 windowHeight = 0;
	//if we're making a full screen window, setup extra settings
	if(type == FULLSCREEN)
	{
		isFullscreen = true;
		//by default, we want to set the screen resolution to the display's current resolution
		clientWidth = displayWidth;
		clientHeight = displayHeight;
		windowWidth	= displayWidth;
		windowHeight = displayHeight;	

		memset(&dmDisplaySettings, 0, sizeof(dmDisplaySettings));
		dmDisplaySettings.dmSize       = sizeof(dmDisplaySettings);
		dmDisplaySettings.dmPelsWidth  = (unsigned long)displayWidth;
		dmDisplaySettings.dmPelsHeight = (unsigned long)displayHeight;
		dmDisplaySettings.dmBitsPerPel = 32;			
		dmDisplaySettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmDisplaySettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		//if we're making a window, set it to default size if the given size is invalid (should be 1024x768).
		if(width <= 0 || height <= 0)
		{
			clientWidth  = DEFAULT_WINDOW_WIDTH;
			clientHeight = DEFAULT_WINDOW_HEIGHT;
		}
		else
		{
			clientWidth  = width;
			clientHeight = height;
		}

		//Calculate window size.
		//Remember that the CLIENT AREA must be the desired size, not just the window!
		RECT windowRect;
		memset(&windowRect, 0, sizeof(RECT));
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = clientWidth;
		windowRect.bottom = clientHeight;

		if(!AdjustWindowRect(&windowRect, winStyle, false))
		{
			LogE("Failed to calculate window bounds!");
			return false;
		}
		windowWidth = windowRect.right - windowRect.left;
		windowHeight = windowRect.bottom - windowRect.top;

		// Place the window in the middle of the screen.
		posX = (displayWidth - windowWidth)  / 2;
		posY = (displayHeight - windowHeight) / 2;
	}

	LogD("Building window");

	// Create the window with the screen settings and get the handle to it.
	//if the window's not fullscreen, make sure the maximize button and resizing's disabled.
	window = CreateWindowEx(WS_EX_APPWINDOW, appName, appName, winStyle,
		posX, posY, windowWidth, windowHeight, NULL, NULL, appInstance, NULL);

	if(window == NULL)
	{
		LogE("Failed to build window!");
		return false;
	}

	//now attach the renderer
	if(!attachRenderer(grpWrapper, window))
	{
		return false;
	}

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	// Hide the mouse cursor.
	ShowCursor(!isFullscreen);
	LogD("Built window");
	return true;
}

IGraphicsWrapper* Win32Platform::BuildGraphicsWrapper(RendererType type)
{
	//not sure if we'll use dynamic allocation
	//if(renderer)
	//{
	//	//if we do, delete any existing renderer here
	//	HandleMgr::DeleteHandle(renderer)
	//}
	IGraphicsWrapper* instance = NULL;
	switch(type)
	{
	case OPEN_GL:
		{
			instance = CustomNew<OGLGrpWrapper>(RENDERER_ALLOC, "RendererAlloc");
			break;
		}
	case DIRECTX:
		{
			instance = NULL;
			break;
		}
	default:
		{
			instance = CustomNew<NullGrpWrapper>(RENDERER_ALLOC, "RendererAlloc");
			break;
		}
	}
	//do function loading if needed
	firstInitRenderer(instance);
	return instance;
}

void Win32Platform::ShutdownGraphicsWrapper(GfxWrapperHandle grpWrapper)
{
	switch(grpWrapper->Type())
	{
	case OPEN_GL:
		//we need to release the rendering context attached to the window.
		if(renderContext)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(renderContext);
			renderContext = NULL;
			((OGLGrpWrapper*)grpWrapper.Ptr())->SetContext(NULL);
		}
		//we also need to release any device context info.
		if(devContext)
		{
			int res = ReleaseDC(window, devContext);
			if(res != 1)
			{
				LogW("ShutdownGraphicsWrapper: device context not released!");
			}
			devContext = NULL;
		}
		break;
	case DIRECTX:
		break;
	default:
		break;
	}
}

//A horrible kludge.
void rebuildInstance(RendererType type, GfxWrapperHandle instance, IPlatform* plat)
{
	if(!plat)
	{
		return;
	}

	//shut down the old instance
	if(instance)
	{
		plat->ShutdownGraphicsWrapper(instance);
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
	if(instance)
	{
		//delete the old data
		LDelete(instance.Ptr());
		//repoint the handle to the new instance!
		HandleMgr::MoveHandle(instance, newInstance);
	}
	else
	{
		//instance built, set its handle
		instance = HandleMgr::RegisterPtr(newInstance);
	}
}

bool Win32Platform::SetViewport(GfxWrapperHandle grpWrapper, WindowHnd vpHnd)
{
	//figure out the type;
	//we'll need to restart the renderer.
	HWND newHwnd = HWNDFromWindowHnd(vpHnd);
	if(newHwnd == NULL)
	{
		return true;
	}
	//Also determine window dimensions.
	RECT wndClientDims;
	if(!GetClientRect(newHwnd, &wndClientDims))
	{
		Win32Helpers::LogLastError();
		return false;
	}
	clientWidth = wndClientDims.right;
	clientHeight = wndClientDims.bottom;

	RendererType gfxType = (grpWrapper != 0) ? grpWrapper->Type() : RendererType::INVALID;
	if(gfxType == RendererType::INVALID)
	{
		return false;
	}
	//shut down the original renderer.
	ShutdownGraphicsWrapper(grpWrapper);
	window = newHwnd;
	//ask the renderer to reinitialize the renderer.
	rebuildInstance(gfxType, grpWrapper, this);
	//and reattach this wrapper to the new viewport.
	return attachRenderer(grpWrapper, window);
}

void Win32Platform::BeginRender(GfxWrapperHandle grpWrapper)
{
	//clear color and depth buffer
	if(grpWrapper)
	{
		grpWrapper->Clear();
	}
}

void Win32Platform::EndRender(GfxWrapperHandle grpWrapper)
{
	//SwapBuffers(devContext);
	switch(grpWrapper->Type())
	{
	case OPEN_GL:
		SwapBuffers(devContext);
		break;
	case DIRECTX:
		SwapBuffers(devContext);
		break;
	default:
		break;
	}
}

bool Win32Platform::oglLoadFunctions(OGLGrpWrapper* grpWrapper, HWND hwnd)
{
	LogD("Loading OpenGL functions");
	//first init the platform's loading functions
	//build a temporary window to load functions through
	//make it a splash screen?
	//OGLGrpWrapper renderer = OGLGrpWrapper();
	int errVal;
	HDC deviceContext;
	PIXELFORMATDESCRIPTOR pixFormat;

	//pull a temporary device context
	deviceContext = GetDC(hwnd);
	if(!deviceContext)
	{
		LogW("Failed to get device context!");
		return false;
	}

	//make a default pixel format so we can load extensions
	errVal = SetPixelFormat(deviceContext, 1, &pixFormat);
	if(errVal != 1)
	{
		LogW("Failed to get pixel format!");
		return false;
	}

	//make a temporary rendering context
	//so we can load the WGL & OpenGL functions
	renderContext = wglCreateContext(deviceContext);
	if(!renderContext)
	{
		LogW("Failed to make render context!");
		return false;
	}
	//and set it as the window's render context
	errVal = wglMakeCurrent(deviceContext, renderContext);
	if(errVal != 1)
	{
		LogW("Failed to set render context!");
		return false;
	}

	//load wgl functions
	errVal = wgl_LoadFunctions(deviceContext);
	//we need the wgl functions to setup render contexts;
	//if the load failed, we have to quit here
	if(errVal == wgl_LOAD_FAILED)
	{
		LogW("Failed to load WGL functions!");
		return false;
	}
	//otherwise, we're almost there
	LogD("Loaded WGL functions");

	//now we can load the renderer's functions
	if(!grpWrapper->LoadFunctions())
	{
		LogW("Failed to load OpenGL functions!");
		return false;
	}
	//release the temporary contexts
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(renderContext);
	renderContext = NULL;
	ReleaseDC(hwnd, deviceContext);
	deviceContext = NULL;

	return true;
}

bool Win32Platform::oglInitRenderer(OGLGrpWrapper* grpWrapper, HWND hwnd, F32 farPlaneDist, F32 nearPlaneDist, bool vSync)
{
	LogD("Initializing OpenGL renderer");
	//int* attributeListInt;
	int pixelFormat[1];
	unsigned int formatCount;
	int errVal;
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	//int* attributeList;
	if(renderContext)
	{
		wglDeleteContext(renderContext);
		renderContext = NULL;
	}

	if(devContext)
	{
		ReleaseDC(hwnd, devContext);
		devContext = NULL;
	}

	//make sure we have a device context
	devContext = GetDC(hwnd);
	if(!devContext)
	{
		LogW("Failed to get device context!");
		return false;
	}

	//now define attributes:
	int attributeListInt[] = {	WGL_SUPPORT_OPENGL_ARB,	TRUE,						//must be able to render OpenGL, of course
								WGL_DRAW_TO_WINDOW_ARB,	TRUE,						//must be able to render to a window
								WGL_ACCELERATION_ARB,	WGL_FULL_ACCELERATION_ARB,	//must support hardware acceleration
								WGL_COLOR_BITS_ARB,		24,							//must support 24 bit color
								WGL_DEPTH_BITS_ARB,		24,							//must support 24 bit depth buffer
								WGL_DOUBLE_BUFFER_ARB,	TRUE,						//must support double buffering for real time rendering
								WGL_SWAP_METHOD_ARB,	WGL_SWAP_EXCHANGE_ARB,		//buffers must swap via exchanging values
								WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,			//must use RGBA valued pixels
								WGL_STENCIL_BITS_ARB,	8,							//must support 8 bit stencil buffer
								//currently optional:
								//multisampling options
								0 };												//attribute list must be null-terminated

	//try to get a matching pixel format
	errVal = wglChoosePixelFormatARB(devContext, attributeListInt, NULL, 1, pixelFormat, &formatCount);
	if(errVal != 1)
	{
		//most likely happens if system doesn't have a good enough graphics card
		//or doesn't have a GPU at all
		LogW("Failed to get pixel format! GPU may be below minimum specs!");
		return false;
	}

	//if successful, the best matching format will be format 0; set it as the current format
	errVal = SetPixelFormat(devContext, pixelFormat[0], &pixelFormatDescriptor);
	if(errVal != 1)
	{
		LogW("Failed to set pixel format!");
		Win32Helpers::LogLastError();
		return false;
	}

	//we want OpenGL 4.0; no support for fallback to 3.3 at the moment
	int attributeList[] = {	WGL_CONTEXT_MAJOR_VERSION_ARB,	OGLGrpWrapper::MIN_MAJOR_VER,
							WGL_CONTEXT_MINOR_VERSION_ARB,	OGLGrpWrapper::MIN_MINOR_VER,
							0};

	//make the rendering context
	renderContext = wglCreateContextAttribsARB(devContext, 0, attributeList);
	if(!renderContext)
	{
		//might fail if card doesn't support 4.0?
		LogW("Failed to make render context! Card might not support GL 4.0!");
		return false;
	}
	//log render context details

	//set rendering context as active context
	errVal = wglMakeCurrent(devContext, renderContext);
	if(errVal != 1)
	{
		LogW("Failed to set render context!");
		return false;
	}
	grpWrapper->SetContext(renderContext);
	//setup non-platform rendering properties
	grpWrapper->SetScreenResolution(Vector2(clientWidth, clientHeight));
	grpWrapper->SetVSyncEnabled(vSync);

	if(!grpWrapper->Startup())
	{
		LogW("Failed to initialize OpenGL API!");
		return false;
	}
	//setup vsync
	if(vSync)
	{
		errVal = wglSwapIntervalEXT(TRUE);
	}
	else
	{
		errVal = wglSwapIntervalEXT(FALSE);
	}
	if(errVal != 1)
	{
		LogW("Failed to set vsync!");
		return false;
	}

	return true;
}
#pragma endregion

LRESULT CALLBACK Win32Platform::WindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		//a controller was moved?
		case WM_INPUT:
		{
			//call the input callback with the message's info
			instance->onUpdateInput(wparam, lparam);
			return DefWindowProc(hwnd, umessage, wparam, lparam);
			//return 0;
		}
		case WM_MOUSEMOVE:
		{
			instance->onUpdateGUI(lparam);
			return 0;
		}
		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		//any other messages should be sent to the default message handler
		//as our application won't make use of them.
		default:
		{
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}

void Win32Platform::ClearTerm()
{
	//a little complex.
	//you need to get the current stdout, 
	//and that gives you access to the term.
	//then fill the term w/ spaces.
	//We can't use ANSI escapes because it's not supported by cmd -
	//Even when it was supported, you had to ensure ANSI.SYS was also loaded.
	//Supposedly works on Linux (not POSIX) terms, though.
	HANDLE stdOut;
	CONSOLE_SCREEN_BUFFER_INFO csBufInfo;
	DWORD charsWritten;
	DWORD numCells;
	//in general, we want to reset the cursor to the upper left = (0,0)
	COORD homeCoords = { 0, 0 };

	//get the terminal for stdout, verify we have stdout at all
	stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(stdOut == INVALID_HANDLE_VALUE)
	{
		return;
	}

	//we also need to know how many characters the screen can display -
	//sys calls these cells
	if(!GetConsoleScreenBufferInfo(stdOut, &csBufInfo))
	{
		return;
	}
	numCells = csBufInfo.dwSize.X * csBufInfo.dwSize.Y;

	//now fill the screen w/ spaces
	if(!FillConsoleOutputCharacter(stdOut, (TCHAR)' ', numCells, homeCoords, &charsWritten))
	{
		return;
	}

	//not done yet - also need to describe the attributes of each cell.
	//if we were filling with visible characters, these could change the colors of the screen
	if(!FillConsoleOutputAttribute(stdOut, csBufInfo.wAttributes, numCells, homeCoords, &charsWritten))
	{
		return;
	}

	//finally, move cursor home
	SetConsoleCursorPosition(stdOut, homeCoords);
}

void* Win32Platform::GetOGLCtx() const
{
	return wglGetCurrentContext();
}

void* Win32Platform::GetHDC() const
{
	return wglGetCurrentDC();
}
#endif //WIN32
