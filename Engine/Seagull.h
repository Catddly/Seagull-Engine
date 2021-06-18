#include "Common/Base/BasicTypes.h"

//----------------------------------------------------------
// System modules
//----------------------------------------------------------
#include "Common/System/ISystem.h"
#include "Common/System/IFileSystem.h"
#include "Common/System/ILog.h"
#include "Common/User/IApp.h"

//----------------------------------------------------------
// Platform specification modules
//----------------------------------------------------------
#include "Common/Platform/ISystemTime.h"
#include "Common/Platform/IOperatingSystem.h"

#include "Common/Thread/IThread.h"
#include "Core/Platform/Windows/WindowsMutex.h"

#include "Common/Memory/IMemory.h"

//----------------------------------------------------------
// Standard template library
//----------------------------------------------------------
#include "Common/Stl/vector.h"
#include "Common/Stl/string.h"
#include "Common/Stl/string_view.h"

//----------------------------------------------------------
// Math library
//----------------------------------------------------------
#include "Common/Math/Vector.h"
#include "Common/Math/Matrix.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "EntryPoint.h"
