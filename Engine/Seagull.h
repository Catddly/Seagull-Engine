//----------------------------------------------------------
// Basic
//----------------------------------------------------------
#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

//----------------------------------------------------------
// System Modules
//----------------------------------------------------------
#include "System/IFileSystem.h"
#include "System/ILogger.h"
#include "User/IApp.h"

//----------------------------------------------------------
// System Implementation
//----------------------------------------------------------
#include "System/System.h"

//----------------------------------------------------------
// Platform Specification Modules
//----------------------------------------------------------
#include "Platform/IOperatingSystem.h"
#include "Platform/SystemTime.h"

#include "Thread/IThread.h"
#include "Thread/IJobSystem.h"

#include "System/IInput.h"
#include "Memory/IMemory.h"

//----------------------------------------------------------
// Render Relative
//----------------------------------------------------------

//----------------------------------------------------------
// System Data
//----------------------------------------------------------
#include "Platform/Window.h"
#include "Platform/OsDevices.h"

//----------------------------------------------------------
// System Utility
//----------------------------------------------------------
#include "Profile/Timer.h"

//----------------------------------------------------------
// Standard Template library
//----------------------------------------------------------
#include "Stl/vector.h"
#include "Stl/string.h"
#include "Stl/string_view.h"
#include "Stl/shared_ptr.h"
#include <EASTL/map.h>

//----------------------------------------------------------
// Math Library
//----------------------------------------------------------
#include "Math/Vector.h"
#include "Math/Matrix.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "EntryPoint.h"