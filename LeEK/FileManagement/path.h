#pragma once

#include "Strings/String.h"
#include <boost/filesystem.hpp>

namespace LeEK
{
	class Path
	{
	private:
		boost::filesystem::path storedPath;
	public:
		Path(String s);
		Path(const char* chr = "");
		~Path(void) {}

		//inline const char* ToCStr() const { return (char*)storedPath.c_str(); }
		inline String ToString() const { return String(storedPath.string().c_str()); }
		inline boost::filesystem::path PathImplementation() const { return storedPath; }

		//path part retrieval
		const String GetExtension() const;
		//gets the filename, if it exists.
		const String GetBaseName() const;
		const String GetDirectory() const;
		//virtual String GetProgramPath() = 0; //technically might have a chance to not be the app's path in implementation, but it's unlikely	
	};
}
