#pragma once

#include "Defs/Defs.h"
#include "Base/TimePoint.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	TimePoint GetFileCreateTime(const char* filename);
	TimePoint GetFileLastWriteTime(const char* filename);
	TimePoint GetFileLastReadTime(const char* filename);

	void TraverseAllFile(const char* folder, FileTraverseFunc func);

}
#endif