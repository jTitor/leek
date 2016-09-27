#pragma once
#include <string>
#include "../LeEK/Logging/Log.h"

using namespace std;

namespace LeEK
{
	class ModelLog
	{
	public: //weird, but need definition before declaring verboseLevel
		enum Verbosity
		{
			ERR		= 0,
			WARN	= 1,
			INFO	= 2,
			DEBUG	= 3,
			VERB	= 4,
			LENGTH	= 5
		};
	private:
		static ModelLog instance;
		ModelLog(void);
		~ModelLog(void);
		static bool bufferingEnabled;
		static bool bufferingPaused;
		static void bufferMessage(string message, Verbosity v);
	public:
		static unsigned int BufferLength();
		static string GetMsg(unsigned int idx);
		static Verbosity GetMsgVerbosity(unsigned int idx);
		static void ForceDumpBuffer();
		static bool BufferEnabled();
		static void SetBufferEnabled(bool val);
		static bool BufferPaused();
		static void SetBufferPaused(bool val);

		//prints the given string and buffers it without any formatting.
		//use only in emergencies, when you can't get a log otherwise!
		static void RAW(const char* message);
		static void LogMessage(string message, Verbosity v);
		//functions for logging messages
		static void E(string message) { LogMessage(message, ERR); }//, tag); }
		static void W(string message) { LogMessage(message, WARN); }//, tag); }
		static void I(string message) { LogMessage(message, INFO); }//, tag); }
		static void D(string message) { LogMessage(message, DEBUG); }//, tag); }
		static void V(string message) { LogMessage(message, VERB); }//, tag); }

		static void E(const char* message) { LogMessage(message, ERR); }//, tag); }
		static void W(const char* message) { LogMessage(message, WARN); }//, tag); }
		static void I(const char* message) { LogMessage(message, INFO); }//, tag); }
		static void D(const char* message) { LogMessage(message, DEBUG); }//, tag); }
		static void V(const char* message) { LogMessage(message, VERB); }//, tag); }

		static void E(LeEK::String message) { LogMessage(message.c_str(), ERR); }//, tag); }
		static void W(LeEK::String message) { LogMessage(message.c_str(), WARN); }//, tag); }
		static void I(LeEK::String message) { LogMessage(message.c_str(), INFO); }//, tag); }
		static void D(LeEK::String message) { LogMessage(message.c_str(), DEBUG); }//, tag); }
		static void V(LeEK::String message) { LogMessage(message.c_str(), VERB); }
	};
}