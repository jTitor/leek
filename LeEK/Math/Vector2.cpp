#include "StdAfx.h"
#include "Vector2.h"

using namespace LeEK;

const Vector2 Vector2::Zero(0.0f, 0.0f);
const Vector2 Vector2::One(1.0f, 1.0f);
const Vector2 Vector2::Up(0.0f, 0.0f);
const Vector2 Vector2::Right(1.0f, 0.0f);

String Vector2::ToString()
const {
	char rowBuf[64];
	
	sprintf_s(rowBuf, sizeof(rowBuf), "< %.4f, %.4f >", x, y);
	return rowBuf;
}

Vector2::~Vector2(void)
{
}
