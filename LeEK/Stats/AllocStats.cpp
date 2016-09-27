#include "AllocStats.h"
#include "DebugUtils/Assertions.h"
#include "Logging/Log.h"
#include "FileManagement/Filesystem.h"
#include "Math/MathFunctions.h"
using namespace LeEK;

size_t ovrhdTot = 0;
size_t wasteTot = 0;
//kinda a weird way to do it; if space permits,
//would rather use a struct indicating frag position and size
size_t fragTot = 0;
I32 fragPieces = 0;
size_t osAllocTot = 0;

IStatDisplayer* ovrhdDisp = NULL;
IStatDisplayer* osAllocDisp = NULL;

const U32 BUF_SIZE = 512;
char lineBuf[BUF_SIZE];

void OverheadStats::_ReportOverhead(size_t size) { ovrhdTot += size; }
void OverheadStats::_RemoveOverhead(size_t size)
{
	L_ASSERT(size <= ovrhdTot && "Invalid overhead removal!");
	ovrhdTot -= size;
}

//Report unused space within an allocation
void OverheadStats::_ReportWaste(size_t size) { wasteTot += size; }
void OverheadStats::_RemoveWaste(size_t size)
{
	//L_ASSERT(size <= wasteTot && "Invalid waste removal!");
	if(wasteTot < size)
	{
		wasteTot = 0;
		return;
	}
	wasteTot -= size;
}
		
//report noncontiguous free blocks
//hard to specify what was removed where,
//so instead you can call to clear the fragmentation stats
void OverheadStats::_ReportFragPiece(size_t size)
{
	fragTot += size;
	fragPieces++;
}
void OverheadStats::_RemoveFragPiece(size_t size)
{
	size = Math::Min(size, fragTot);
	L_ASSERT(size <= fragTot && fragPieces > 0 && "Invalid waste removal!");
	fragTot -= size;
	fragPieces--;
}

//stat displaying funcs
void OverheadStats::SetStatDisplayer(IStatDisplayer* displayer)
{
	if(displayer)
	{
		ovrhdDisp = displayer;
	}
}
void OverheadStats::WriteStats()
{
	L_ASSERT(ovrhdDisp && "No output callback set");
	ovrhdDisp->WriteStatLn("Overhead Info(kB):");
	ovrhdDisp->WriteStatLn("Ovrhd\t| Waste\t| FragTot\t| FragPieces");
	sprintf_s(	lineBuf, BUF_SIZE, "%.3f\t| %.3f\t| %.3f\t| %d",
				((F64)ovrhdTot) / 1024, ((F64)wasteTot) / 1024, ((F64)fragTot) / 1024, fragPieces);
	ovrhdDisp->WriteStatLn(lineBuf);
}
void OverheadStats::WriteCSV(DataStream* file)
{
	file->WriteLine("Overhead(kB),Waste(kB),Frag Total(kB),Frag Pieces");
	sprintf_s(	lineBuf, BUF_SIZE, "%.3f,%.3f,%.3f,%d",
				((F64)ovrhdTot) / 1024, ((F64)wasteTot) / 1024, ((F64)fragTot) / 1024, fragPieces);
	file->WriteLine(lineBuf);
}
void OverheadStats::DumpCSV(const Path& path)
{
	//read through all the tags, and write the data in csv
	LogD("Dumping profiler data...");
	//open up a file, of course
	DataStream* logFile = Filesystem::OpenFile(path);
	if(!logFile)
	{
		LogW("Couldn't open stats log file!");
		return;
	}
	//try to append to any existing log
	logFile->SeekToEnd();
	if(logFile->FileSize() > 0)
	{
		logFile->WriteLine();
	}
	WriteCSV(logFile);
	//we're done, remember to close the file
	logFile->Close();
	LogD("Dump complete.");
}

void AllocStats::_ReportOSAlloc(size_t size) { osAllocTot += size; }
void AllocStats::_RemoveOSAlloc(size_t size) { osAllocTot -= size; }

void AllocStats::SetStatDisplayer(IStatDisplayer* displayer)
{
	if(displayer)
	{
		osAllocDisp = displayer;
	}
}
void AllocStats::WriteStats()
{
	L_ASSERT(osAllocDisp && "No output callback set");
	sprintf_s(lineBuf, BUF_SIZE, "OS Page Allocs(kB): %.3f", ((F64)osAllocTot) / 1024);
	osAllocDisp->WriteStatLn(lineBuf);
}
void AllocStats::WriteCSV(DataStream* file)
{
	sprintf_s(lineBuf, BUF_SIZE, "OS Page Allocs(kB)\n%.3f", ((F64)osAllocTot) / 1024);
	file->WriteLine(lineBuf);
}
void AllocStats::DumpCSV(const Path& path)
{
	//read through all the tags, and write the data in csv
	LogD("Dumping profiler data...");
	//open up a file, of course
	DataStream* logFile = Filesystem::OpenFile(path);
	if(!logFile)
	{
		LogW("Couldn't open stats log file!");
		return;
	}
	//try to append to any existing log
	logFile->SeekToEnd();
	if(logFile->FileSize() > 0)
	{
		logFile->WriteLine();
	}
	WriteCSV(logFile);
	//we're done, remember to close the file
	logFile->Close();
	LogD("Dump complete.");
}