#include "ModelLog.h"
#include "../LeEK/Time/DateTime.h"
#include "../LeEK/Logging/Log.h"
#include <vector>

using namespace LeEK;

ModelLog ModelLog::instance = ModelLog();

ModelLog::Verbosity int2Verb(int verbosity)
{
	if(verbosity >= 0 && verbosity <= ModelLog::Verbosity::LENGTH)
	{
		return (ModelLog::Verbosity)verbosity;
	}
	return ModelLog::Verbosity::ERR;
}

void onLibraryLogMessage(const char* const msg, int verbosity)
{
	ModelLog::LogMessage("[LeEKLib] " + string(msg), int2Verb(verbosity));
}

//intercept messages from the main engine's log
bool ModelLog::bufferingEnabled = true;
bool ModelLog::bufferingPaused = false;

typedef pair<ModelLog::Verbosity, string> LogElemPair;
typedef vector<LogElemPair> LogVec;

LogVec buffer = LogVec();

void ModelLog::bufferMessage(string message, ModelLog::Verbosity verb)
{
	//now put message in buffer
	buffer.push_back(LogElemPair(verb, message));
}

ModelLog::ModelLog(void)
{
	Log::SetLogMessageHandler(onLibraryLogMessage);
}

ModelLog::~ModelLog(void)
{
}

unsigned int ModelLog::BufferLength()
{
	return buffer.size();
}

string ModelLog::GetMsg(unsigned int idx)
{
	if(idx > buffer.size())
	{
		return "";
	}
	return buffer[idx].second;
}

ModelLog::Verbosity ModelLog::GetMsgVerbosity(unsigned int idx)
{
	if(idx > buffer.size())
	{
		return ERR;
	}
	return buffer[idx].first;
}

//Opens a log file at the program's directory, plus any given subdirectory.
//If passing a subdirectory, prepend a "/", and do NOT append a "/".

void ModelLog::ForceDumpBuffer()
{
	buffer.clear();
}

bool ModelLog::BufferEnabled()
{
	return bufferingEnabled;
}

void ModelLog::SetBufferEnabled(bool val)
{
	bufferingEnabled = val;
}

bool ModelLog::BufferPaused()
{
	return bufferingPaused;
}

void ModelLog::SetBufferPaused(bool val)
{
	bufferingPaused = val;
}

void ModelLog::RAW(const char* message)
{
	if(bufferingEnabled && !bufferingPaused)
	{
		bufferMessage(message, ModelLog::Verbosity::ERR);
	}
}

void ModelLog::LogMessage(string message, Verbosity v)//, const char* tag)
{
	if(bufferingEnabled && !bufferingPaused)
	{
		string bufMsg = "[" + string(DateTime::GetCurrLocalTime()) +  "] " + 
						message + "\n";
		bufferMessage(bufMsg, v);
	}
}
