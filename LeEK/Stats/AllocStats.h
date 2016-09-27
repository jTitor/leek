#pragma once
#include "Datatypes.h"
#include "FileManagement/DataStream.h"
#include "FileManagement/path.h"
#include "Stats/IStatDisplayer.h"

namespace LeEK
{
#if defined(_DEBUG) || defined(RELDEBUG)
#define ENABLE_ALLOC_STATS
#endif
	//functions to monitor dynamic alloc overheads.
	//There are three categories of overhead:
	//	* Overhead from allocation headers
	//	* Unused space within allocations, 
	//	  such as in a pool allocator
	//	* Fragmentation from noncontiguous blocks
	namespace OverheadStats
	{
		//TODO: add getters

		//Report any allocations required by allocation subsystems -
		//headers for pages and RBTree nodes, for example
		void _ReportOverhead(size_t size);
		void _RemoveOverhead(size_t size);

		//Report unused space within an allocation
		void _ReportWaste(size_t size);
		void _RemoveWaste(size_t size);
		
		//report noncontiguous free blocks
		//hard to specify what was removed where,
		//so instead you can call to clear the fragmentation stats
		void _ReportFragPiece(size_t size);
		void _RemoveFragPiece(size_t size);

		//stat displaying funcs
		void SetStatDisplayer(IStatDisplayer* displayer);
		void WriteStats();
		void WriteCSV(DataStream* file);
		void DumpCSV(const Path& path);
	}

#ifdef ENABLE_ALLOC_STATS
#define ReportOverhead(SIZE) OverheadStats::_ReportOverhead(SIZE)
#define RemoveOverhead(SIZE) OverheadStats::_RemoveOverhead(SIZE)
#define ReportWaste(SIZE) OverheadStats::_ReportWaste(SIZE)
#define RemoveWaste(SIZE) OverheadStats::_RemoveWaste(SIZE)
#define ReportFragPiece(SIZE) OverheadStats::_ReportFragPiece(SIZE)
#define RemoveFragPiece(SIZE) OverheadStats::_RemoveFragPiece(SIZE)
#else
#define ReportOverhead(SIZE)
#define RemoveOverhead(SIZE)
#define ReportWaste(SIZE)
#define RemoveWaste(SIZE)
#define ReportFragPiece(SIZE)
#define RemoveFragPiece(SIZE)
#endif

	namespace AllocStats
	{
		//TODO: add getters

		void _ReportOSAlloc(size_t size);
		void _RemoveOSAlloc(size_t size);

		void SetStatDisplayer(IStatDisplayer* displayer);
		void WriteStats();
		void WriteCSV(DataStream* file);
		void DumpCSV(const Path& path);
	}

#ifdef ENABLE_ALLOC_STATS
#define ReportOSAlloc(SIZE) AllocStats::_ReportOSAlloc(SIZE)
#define RemoveOSAlloc(SIZE) AllocStats::_RemoveOSAlloc(SIZE)
#else
#define ReportOSAlloc(SIZE)
#define RemoveOSAlloc(SIZE)
#endif
}