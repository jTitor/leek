#include "Color.h"

using namespace LeEK;

String Color::ToString() const
{
	char rowBuf[128];
	sprintf_s(rowBuf, sizeof(rowBuf), "< %.4f, %.4f, %.4f, %.4f >", values[0], values[1], values[2], values[3]);
	return rowBuf;
}