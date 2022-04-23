#ifndef NOMINMAX
#	define NOMINMAX
#endif

//----------------------------------------------------------
// Basic
//----------------------------------------------------------
#include "Defs/Defs.h"
#include "Base/BasicTypes.h"
#include "Base/Handle.h"

//----------------------------------------------------------
// System Modules
//----------------------------------------------------------
#include "System/FileSystem.h"
#include "System/Logger.h"
#include "User/IApp.h"

//----------------------------------------------------------
// System Implementation
//----------------------------------------------------------
#include "System/System.h"

//----------------------------------------------------------
// Platform Specification Modules
//----------------------------------------------------------
#include "Platform/OS.h"
#include "Platform/SystemTime.h"

#include "Thread/Thread.h"
#include "Thread/IJobSystem.h"

#include "System/Input.h"
#include "Memory/Memory.h"

//----------------------------------------------------------
// Render Relative
//----------------------------------------------------------

//----------------------------------------------------------
// System Utility
//----------------------------------------------------------
#include "Profile/Timer.h"
#include "Profile/Profile.h"

//----------------------------------------------------------
// Standard Template library
//----------------------------------------------------------
#include "Stl/vector.h"
#include "Stl/string.h"
#include "Stl/string_view.h"
#include "Stl/SmartPtr.h"
#include <EASTL/map.h>

//----------------------------------------------------------
// Math Library
//----------------------------------------------------------
#include "Math/MathBasic.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "EntryPoint.h"