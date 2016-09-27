#include "StdAfx.h"
#include "PerformanceTests.h"
#include <FileManagement/Filesystem.h>
#include <FileManagement/DataStream.h>
#include <Time/GameTime.h>
#include <iostream>

using namespace LeEK;

void PerformanceTests::TestLargeFileLoad()
{
	//we're going to assume there's a copy of taurus.dae in the app's folder
	Path filePath("./TestFiles/LargeFileReading/taurus.dae");
	GameTime timer = GameTime();
	//prime clock
	timer.Tick();
	//now load the file - do not display it!
	DataStream* file = Filesystem::OpenFile(filePath);
	if(!file)
	{
		std::cout << "Could not open " << filePath.GetBaseName() << "!\n";
		return;
	}
	char* data = file->ReadAll();
	timer.Tick();
	std::cout << "Read " << filePath.GetBaseName() << " in " << timer.ElapsedGameTime().ToSeconds() << "s.\n";
	//DataStream file = Filesystem::OpenFile
}
