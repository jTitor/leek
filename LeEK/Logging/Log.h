#pragma once
#include "Datatypes.h"
#include "Strings/String.h"

namespace LeEK
{
	//Forward decs.
	class IPlatform;

	//should logging be allowed in this build?
#define LOG_ENABLED 1

	//Global logging class. 
	//Sends all messages sent to it to a log file 
	//and can selectively display messages to a logging console.
	//By default, displays error, warning, and information messages, 
	//but can adjust to display lower priority messages as well.
	//
	//Verbosity priorities are, from highest to lowest:
	//	* Errors
	//	* Warnings
	//	* Info
	//	* Debug
	//	* Verbose
	class Log
	{
	public: //weird, but need definition before declaring verboseLevel
		enum Verbosity
		{
			ERR		= 0,
			WARN	= 1,
			INFO	= 2,
			DEBUG	= 3,
			VERB	= 4
		};
	private:
		IPlatform* plat;
		Log(void);
		~Log(void);
		static Verbosity verboseLevel;
		//static const char* tagFilter;
		static void logMessage(const char* const message, Verbosity v);//, const char* tag);
		//function pointer that libraries can use to intercept messages from Log.
		static void (*logMessageHandler)(const char* const, int);
		static void onLogMessage(const char* const message, Verbosity v);
		//static void bufferMessage(const char* message);
	public:
		static void OpenLogFile(const char* logSubDir);
		static void CloseLogFile();
		static void ForceDumpBuffer();
		static bool BufferEnabled();
		static void SetBufferEnabled(bool val);
		static bool BufferPaused();
		static void SetBufferPaused(bool val);
		static void SetVerbosity(Verbosity v);
		static Verbosity GetVerbosity() { return verboseLevel; }
		static void SetLogMessageHandler(void (*newLogMessageHandler)(const char* const, int));
		//static void SetTagFilter(const char* tag);
		//static void ClearTagFilter() { SetTagFilter(""); }
		//static const char* GetTagFilter() { return tagFilter; }

		//prints the given string and buffers it without any formatting.
		//use only in emergencies, when you can't get a log otherwise!
		static void RAW(const char* message);

		//functions for logging messages
		static void E(const String& message/*, const char* tag = ""*/) { logMessage(message.c_str(), ERR); }//, tag); }
		static void W(const String& message) { logMessage(message.c_str(), WARN); }//, tag); }
		static void I(const String& message) { logMessage(message.c_str(), INFO); }//, tag); }
		static void D(const String& message) { logMessage(message.c_str(), DEBUG); }//, tag); }
		static void V(const String& message) { logMessage(message.c_str(), VERB); }//, tag); }

		static void E(const char* message) { logMessage(message, ERR); }//, tag); }
		static void W(const char* message) { logMessage(message, WARN); }//, tag); }
		static void I(const char* message) { logMessage(message, INFO); }//, tag); }
		static void D(const char* message) { logMessage(message, DEBUG); }//, tag); }
		static void V(const char* message) { logMessage(message, VERB); }//, tag); }
	};

	//really wacky, but it'd be useful to be able to strip logging out of the build
#if LOG_ENABLED
	//#define RAW(STR) Log::RAW(STR)
	#define LogE(STR) Log::E(STR)
	#define LogW(STR) Log::W(STR)
	#define LogI(STR) Log::I(STR)
	#define LogD(STR) Log::D(STR)
	#define LogV(STR) Log::V(STR)
#else
	#define LogE(STR)
	#define LogW(STR)
	#define LogI(STR)
	#define LogD(STR)
	#define LogV(STR)
#endif
}
