#include <StdAfx.h>
#include "path.h"

using namespace LeEK;

Path::Path(String s) : storedPath(s.c_str())
{
}

Path::Path(const char* chr)
{
	storedPath = boost::filesystem::path(chr);
}

/*
const String Path::ToString()
{
	return storedPath.string();
}*/

const String Path::GetExtension()
const {
	return String((char*)storedPath.extension().string().c_str());
}

const String Path::GetBaseName()
const {
	return String((char*)storedPath.filename().string().c_str());//string().c_str();
}

const String Path::GetDirectory() const
{
	return String((char*)storedPath.parent_path().string().c_str());
}