#include "DateTime.h"

using namespace LeEK;


struct tm* now;
const U32 MAX_DATE_LEN = 16;
char dateBuf [MAX_DATE_LEN];

void updateDateTime()
{
	//get current raw time
	time_t rawTime;
	time(&rawTime);
	//convert to a local time
	//needs struct at the front? dunno why
	now = localtime(&rawTime);
}

char* DateTime::GetCurrDate()
{
	updateDateTime();
	//date format is MM-DD-YYYY
	strftime(dateBuf, MAX_DATE_LEN*sizeof(char), "%m-%d-%Y", now);
	return dateBuf;
}

char* DateTime::GetCurrLocalTime()
{
	updateDateTime();
	//time format is HH:MM:SS
	//note that hours are in 24 hour format!
	strftime(dateBuf, MAX_DATE_LEN*sizeof(char), "%H:%M:%S", now);
	return dateBuf;
}