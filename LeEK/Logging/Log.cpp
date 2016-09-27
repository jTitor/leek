#include "Log.h"
#include "FileManagement/Filesystem.h"
#include "FileManagement/DataStream.h"
#include "Time/DateTime.h"
#include "Platforms/IPlatform.h"
#include <iostream>

using namespace LeEK;

Log::Verbosity Log::verboseLevel = INFO;
void (*Log::logMessageHandler)(const char* const, int) = NULL;
//const char* Log::tagFilter = "";
bool bufferingEnabled = true;
bool bufferingPaused = false;
DataStream* logFile;
//buffer vars
const U32 MAX_BUF_CHARS = 1024;
U32 bufPos = 0;
char logBuffer[MAX_BUF_CHARS] = {0};
const char* FALLBACK_LOG_DIR = "/logs";

void dumpBuffer()
{
	if(logFile)
	{
		logFile->Write(logBuffer, bufPos);
	}
	bufPos = 0;
}

void Log::onLogMessage(const char* const msg, Log::Verbosity v)
{
	if(logMessageHandler != NULL)
	{
		logMessageHandler(msg, v);
	}
}

void bufferMessage(const char* message, U32 messageLen)
{
	//make sure this message can even fit in the buffer
	if(messageLen > MAX_BUF_CHARS)
	{
		//if not, dump the buffer and write straight to the file
		dumpBuffer();
		if(logFile)
		{
			logFile->Write(message, messageLen);
		}
		return;
	}

	//do we have space?
	if(bufPos+messageLen >= MAX_BUF_CHARS)
	{
		//if not, dump buffer
		dumpBuffer();
	}
	//now put message in buffer
	strcpy_s(logBuffer + bufPos, MAX_BUF_CHARS - bufPos, message);
	bufPos += messageLen;
}

//Opens a log file at the program's directory, plus any given subdirectory.
//If passing a subdirectory, prepend a "/", and do NOT append a "/".
void Log::OpenLogFile(const char* logSubDir)
{
	//kind of a pain, but we need to be able to have a file we can write to
	//independent of the allocation system
	//Strings depend on it, so this means no using strings here
	if(!Filesystem::PlatformSet())
	{
		SetBufferEnabled(false);
		Log::RAW("---Platform unavailable, disabling log buffering!---\n");
		return;
	}
	const U32 bufCount = 1024;
	const U32 bufSize = bufCount*sizeof(char);
	char buf[bufCount];
	strcpy_s(buf, sizeof(buf), Filesystem::GetProgDir());
	strcat_s(buf, sizeof(buf), logSubDir);
	strcat_s(buf, sizeof(buf), "/Log - ");
	strcat_s(buf, sizeof(buf), DateTime::GetCurrDate());
	strcat_s(buf, sizeof(buf), ".log");
	Log::RAW((String("Opening file ") + buf + "\n").c_str());
	Path bufPath(buf);
	logFile = Filesystem::OpenFileText(bufPath);
	if(logFile)
	{
		logFile->SeekToEnd();
		if(logFile->FileSize() > 0)
		{
			logFile->Write("\n");
		}
	}
	//if we can't open the file, notify and stop buffering
	else
	{
		SetBufferEnabled(false);
		Log::RAW("---Could not open log file, disabling log buffering!---\n");
	}
}

void Log::CloseLogFile()
{
	if(logFile)
	{
		dumpBuffer();
		logFile->Close();
	}
}

void Log::ForceDumpBuffer()
{
	if(logFile)
	{
		dumpBuffer();
	}
	else
	{
		Log::OpenLogFile(FALLBACK_LOG_DIR);
		dumpBuffer();
	}
}

bool Log::BufferEnabled()
{
	return bufferingEnabled;
}

void Log::SetBufferEnabled(bool val)
{
	bufferingEnabled = val;
}

bool Log::BufferPaused()
{
	return bufferingPaused;
}

void Log::SetBufferPaused(bool val)
{
	bufferingPaused = val;
}

void Log::SetLogMessageHandler(void (*newOnLogMessage)(const char* const, int))
{
	logMessageHandler = newOnLogMessage;
}

void Log::RAW(const char* message)
{
	std::cout << message;
	if(bufferingEnabled && !bufferingPaused)
	{
		bufferMessage(message, strlen(message));
	}
	onLogMessage(message, Log::ERR);
}

void Log::logMessage(const char* const message, Verbosity v)
{
	if(verboseLevel >= v)
	{
		static const char* severPrefix = "";
		//preface with an appropriate header
		switch(v)
		{
		case ERR:
			severPrefix = "E: ";
			break;
		case WARN:
			severPrefix = "W: ";
			break;
		case INFO:
			severPrefix = "I: ";
			break;
		case DEBUG:
			severPrefix = "D: ";
			break;
		case VERB:
			severPrefix = "V: ";
			break;
		default:
			break;
		}
		//display the message
		std::cout << severPrefix << message << "\n";
		//if we're saving to a file, prep for buffering
		if(bufferingEnabled && !bufferingPaused)
		{
			String bufMsg = "[" + String(DateTime::GetCurrLocalTime()) +  "] " + 
							severPrefix + message + "\n";
			bufferMessage(bufMsg.c_str(), bufMsg.length());
		}
	}
	onLogMessage(message, v);
}

void Log::SetVerbosity(Verbosity v)
{
	verboseLevel = v;
}
