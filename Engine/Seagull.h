#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

//----------------------------------------------------------
// Standard template library
//----------------------------------------------------------
#include "Stl/vector.h"
#include "Stl/string.h"
#include "Stl/string_view.h"
#include "Stl/shared_ptr.h"
#include <EASTL/map.h>

//----------------------------------------------------------
// Math library
//----------------------------------------------------------
#include "Math/Vector.h"
#include "Math/Matrix.h"

//----------------------------------------------------------
// System modules
//----------------------------------------------------------
#include "System/ISystem.h"
#include "System/IFileSystem.h"
#include "System/ILogger.h"
#include "User/IApp.h"

//----------------------------------------------------------
// System implementation
//----------------------------------------------------------
#include "Core/Private/System/System.h"

//----------------------------------------------------------
// Platform specification modules
//----------------------------------------------------------
#include "Platform/IOperatingSystem.h"
#include "Platform/SystemTime.h"

#include "Thread/IThread.h"
#include "Thread/IJobSystem.h"

#include "System/IInput.h"
#include "Memory/IMemory.h"

//----------------------------------------------------------
// Render relative
//----------------------------------------------------------

//----------------------------------------------------------
// System data
//----------------------------------------------------------
#include "Platform/Window.h"
#include "Platform/OsDevices.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "EntryPoint.h"