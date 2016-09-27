#ifdef __linux__
#include "LinuxPlatform.h"
#include <unistd.h>
#include "../GraphicsWrappers/NullGrpWrapper.h"
#include "../GraphicsWrappers/OGLGrpWrapper.h"
#include<stdio.h>
#include<stdlib.h>
//header for virtual keys
#include<X11/keysym.h>
//#include<X11/extensions/xf86vmode.h>
#include<GL/gl.h>
//also include hardware acceleration
#include<GL/glx.h>
using namespace LeEK;

const U32 MAX_PATH_LEN = 256;
char progPath[MAX_PATH_LEN] = {0};

//function pointer defs for OpenGL.
typedef GLXContext (*GLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension)

{

  const char *start;
  const char *where, *terminator;

  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if ( where || *extension == '\0' )
    return false;

  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for ( start = extList; ; ) {
    where = strstr( start, extension );

    if ( !where )
      break;

    terminator = where + strlen( extension );

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;

    start = terminator;
  }

  return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

U8 LinuxPlatform::getUniversalKeyCode(U8 keyCode)
{
	//TODO
	return 0;
}

void LinuxPlatform::GenRandomBytes(void* destBuf, U32 bufSizeBytes)
{
	//TODO
}
const char* LinuxPlatform::FilesystemPathSeparator()
{
	//TODO
	return "/";
}

const char* LinuxPlatform::FindFullProgPath()
{
	//TODO
	if(progPath[0] == 0)
	{
		readlink("/proc/self/exe", progPath, MAX_PATH_LEN);
		char* dirEndPos = strrchr(progPath, '/');
		dirEndPos[0] = 0;
	}
	return progPath;
}

const char* LinuxPlatform::GetProgDir()
{
	//TODO
	return FindFullProgPath();
}

const PlatformType LinuxPlatform::Type()
{
	//TODO
	return PlatformType::GNU;
}

//template<class T : IPlatform> static void SetPlatform(T platform) {  }
//static void InitInstance(IPlatform* plat);
bool LinuxPlatform::Startup()
{
	//TODO
	progPath[0] = 0;
	return true;
}

//handles messages from OS. Should return true if OS is telling the program to quit, false otherwise
bool LinuxPlatform::UpdateOS()
{
	//TODO
	return false;
}

void LinuxPlatform::Shutdown()
{
	//TODO
}

void LinuxPlatform::SetInputManager(Handle inMgrHnd)
{
	//TODO
}

void LinuxPlatform::ListInputDevices()
{
	//TODO
	//In Unix, devices are pseudofiles,
	//so we get device events by reading the device's file.
	//This unfortunately means a lot of file I/O work, but it's necessary.
	//Mice under /dev/input/mouse(n),
	//Non-mice and keyboards under /dev/input/event(n).
	//This includes mouse wheels, so you'll be doing some HID-style checks.
	//USB HID apparently under /usb/hid/hiddev(n).
}

//also has counter to get the number of devices
TypedHandle<InputDevInfo> LinuxPlatform::FindInputDevList(U32* numDevs)
{
	//TODO
	return 0;
}

void LinuxPlatform::BuildHIDDetailList()
{
	//TODO
}

void LinuxPlatform::RegisterAllInputDevices()
{
	//TODO
}

//graphics wrapper ops
bool LinuxPlatform::GetWindow(GfxWrapperHandle grpWrapper, WindowType type, U32 width, U32 height)
{
	//The root window (desktop).
	Window                  root;
	XVisualInfo             *visInf;
	XSetWindowAttributes    sWinAttribs;
	XWindowAttributes       winAttribs;
	U32 posX, posY;

	//OpenGL fields.
	GLXFBConfig* fbConf;

	//First, build a window.
	//Tell X we want to draw to this computer's screen
	disp = XOpenDisplay(NULL);
	if(disp == NULL)
	{
		LogE("Can't connect to X Server!");
		return false;
	}
	//Now get a link to the root window of the screen;
	//We'll need to attach our window to it as a child
	root = DefaultRootWindow(disp);

	// Give the application a name.
	char* appName = "Engine";

	//if we have a renderer, call a renderer specific startup function
	if(grpWrapper)
	{
		switch(grpWrapper->Type())
		{
		case OPEN_GL:
			{
				LogD("Entering OpenGL setup");
				//Attribute list.
				//Describes the properties
				//we need in a graphics context.
				//Must be terminated by None.
				GLint glAttribs[] = { 	GLX_X_RENDERABLE,	True,			//This *is* a window for a human to view...
										GLX_DRAWABLE_TYPE,	GLX_WINDOW_BIT,	//Want to draw to a window if possible
										GLX_RENDER_TYPE,	GLX_RGBA_BIT,	//Must be RGBA pixels
										GLX_RED_SIZE,		8,				//Going for 32-bit color
										GLX_GREEN_SIZE,		8,
										GLX_BLUE_SIZE,		8,
										GLX_ALPHA_SIZE,		8,
										GLX_DEPTH_SIZE, 	24,
										GLX_STENCIL_SIZE, 	8,
										GLX_DOUBLEBUFFER, 	True,			//Needs double buffering for no tearing
										GLX_CONFIG_CAVEAT,	GLX_NONE,		//And we want something that's conformant and hardware accelerated
										None };

				//Find a compatible visual mode.
				//The function necessary for this appeared in GLX 1.3;
				//check for version.
				int glxMaj, glxMin;
				if ( !glXQueryVersion( disp, &glxMaj, &glxMin ) ||
					   ( ( glxMaj == 1 ) && ( glxMin < 3 ) ) || ( glxMaj < 1 ) )
				{
					Log::E("GLX version is below 1.3!");
					return false;
				}

				int numConfigs;
				fbConf = glXChooseFBConfig(disp, DefaultScreen(disp), glAttribs, &numConfigs);
				//If there's no compatible mode, give up
				if(fbConf == NULL || numConfigs == 0)
				{
					Log::E("Failed to get visual mode! GPU may be below minimum specs!");
					return false;
				}

				//The mode list is sorted first by caveat,
				//then by color bit depth, then by buffer size,
				//and finally by double buffer.
				//We may want to resort by multisampling level,
				//since almost all of these are specified in glAttribs.

				//In any case, select the best config.
				visInf = glXGetVisualFromFBConfig(disp, fbConf[0]);
				root = RootWindow(disp, visInf->screen);

				//NOTE: Windows stuff. still necessary?
				//make a temp window for OpenGL setup
				//hide the window
				//now run OpenGL setup!
				//now release the window
				break;
			}
		//otherwise, show a blank screen for now
		default:
			LogW("Entering null API setup");
			//TODO
			return false;
			break;
		}
	}

	//Generate the colormap and specify window attributes
	cMap = XCreateColormap(disp, root, visInf->visual, AllocNone);
	sWinAttribs.colormap = cMap;
	sWinAttribs.background_pixmap = None;
	//Presumably windowing manager handles the actual border?
	sWinAttribs.border_pixel = 0;
	//Need to know about window movement and destruction
	sWinAttribs.event_mask = StructureNotifyMask;

	//Specify window details, necessary to calculate proper window size.
	I32 windowWidth = 0;
	I32 windowHeight = 0;
	//if we're making a full screen window, setup extra settings
	if(type == FULLSCREEN)
	{
		//by default, we want to set the screen resolution to the display's current resolution

		// Change the display settings to full screen.

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		//if we're making a window, set it to default size if the given size is invalid (should be 1024x768).
		if(width <= 0 || height <= 0)
		{
			windowWidth = 1024;
			windowHeight = 768;
		}
		else
		{
			windowWidth = 1024;
			windowHeight = 768;
		}

		//Calculate window size.
		//Remember that the CLIENT AREA must be the desired size, not just the window!

		// Place the window in the middle of the screen.
		//posX = (displayWidth - windowWidth)  / 2;
		//posY = (displayHeight - windowHeight) / 2;
	}

	LogD("Building window");
	// Create the window with the screen settings and get the handle to it.
	//if the window's not fullscreen, make sure the maximize button and resizing's disabled.

	win = XCreateWindow(disp, root,
						posX, posY,
						windowWidth, windowHeight,
						0,
						visInf->depth,
						InputOutput,
						visInf->visual,
						CWBorderPixel | CWColormap | CWEventMask,
						&sWinAttribs);

	if(win == NULL)
	{
		LogE("Failed to build window!");
		return false;
	}

	//Free the visual info!
	XFree(visInf);

	//Set the window name...
	XStoreName(disp, win, appName);

	//Map the window...
	XMapWindow(disp, win);

	//now finish initializing the graphics context and attach the wrapper
	if(grpWrapper)
	{
		switch(grpWrapper->Type())
		{
		case OPEN_GL:
			{
				//may need to switch out the X error handler
				//We need at least OpenGL 3.3;
				//if this call fails or isn't defined,
				//then this version of GLX is too old anyway
				int contextAttribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, OGLGrpWrapper::MIN_MAJOR_VER,
										GLX_CONTEXT_MINOR_VERSION_ARB, OGLGrpWrapper::MIN_MINOR_VER,
										None};

				String minVer = StrFromVal(OGLGrpWrapper::MIN_MAJOR_VER) +
												"." +
												OGLGrpWrapper::MIN_MINOR_VER;

				// Get the default screen's GLX extension list
				const char *glxExts = glXQueryExtensionsString( disp,
														  	  	DefaultScreen( disp ) );

				//might have to load function here.
				GLXCREATECONTEXTATTRIBSARBPROC makeContext = (GLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
				//glXCreateContextAttribsARB = (GLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
				if(!isExtensionSupported( glxExts, "GLX_ARB_create_context" ) || !makeContext)//glXCreateContextAttribsARB)
				{
					LogE(String("Couldn't find context generating function! Card might not support GL ") + minVer +	"!");
					return false;
				}
				GLXContext ctx = makeContext(disp, fbConf[0], 0,
																True, contextAttribs);
				//don't forget to free the lists we got!
				XFree(fbConf);

				//Process (or at this point, throw away really) any errors
				XSync(disp, False);

				if(ctx)
				{
					LogV(String("Built GL ") + minVer +	" context");
				}
				else
				{
					LogE(String("Failed to make render context! Card might not support GL ") + minVer +	"!");
					return false;
				}

				//we REALLY want a direct-to-hardware context,
				//not sure if not getting one is a failure
				if(!glXIsDirect(disp, ctx))
				{
					LogW("GL context does not have direct access to hardware!");
				}

				//and finally attach the context to the window!
				glXMakeCurrent(disp, win, ctx);

				break;
			}
		//otherwise, show a blank screen for now
		default:
			break;
		}
	}

	// Bring the window up on the screen and set it as main focus.

	// Hide the mouse cursor.
	LogD("Built window");
	return true;
}

GfxWrapperHandle LinuxPlatform::BuildGraphicsWrapper(RendererType type)
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
				return HandleMgr::RegisterPtr(instance);
			}
		default:
			{
				instance = CustomNew<NullGrpWrapper>(RENDERER_ALLOC, "RendererAlloc");
				return HandleMgr::RegisterPtr(instance);
			}
		}
}

void LinuxPlatform::ShutdownGraphicsWrapper(GfxWrapperHandle grpWrapper)
{
	switch(grpWrapper->Type())
	{
	case OPEN_GL:
		{
			//TODO
			GLXContext ctx = glXGetCurrentContext();
			//we need to release the rendering context attached to the window.
			glXMakeCurrent(disp, 0, 0);
			//we also need to release any device context info.
			glXDestroyContext(disp, ctx);
			break;
		}
	default:
		{
			break;
		}
	}
	//and shutdown the window!
	XDestroyWindow(disp, win);
	XFreeColormap(disp, cMap);
	XCloseDisplay(disp);
}

void LinuxPlatform::BeginRender(GfxWrapperHandle grpWrapper)
{
	if(grpWrapper)
	{
		grpWrapper->Clear();
	}
}

void LinuxPlatform::EndRender(GfxWrapperHandle grpWrapper)
{
	switch(grpWrapper->Type())
	{
	case OPEN_GL:
		glXSwapBuffers(disp, win);
		break;
	default:
		break;
	}
}

//audio ops
IAudioWrapper* LinuxPlatform::BuildAudioWrapper(AudioType type)
{
	//TODO
	return NULL;
}

void LinuxPlatform::ShutdownAudioWrapper(IAudioWrapper* audio)
{
	//TODO
}
#endif //__linux__