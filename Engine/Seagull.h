#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

//----------------------------------------------------------
// System modules
//----------------------------------------------------------
#include "Common/System/ISystem.h"
#include "Common/System/IFileSystem.h"
#include "Common/System/ILog.h"
#include "Common/User/IApp.h"

//----------------------------------------------------------
// System implementation
//----------------------------------------------------------
#include "Core/System/System.h"

//----------------------------------------------------------
// Platform specification modules
//----------------------------------------------------------
#include "Common/Platform/IOperatingSystem.h"
#include "Common/Platform/SystemTime.h"

#include "Common/Thread/IThread.h"
#include "Common/Thread/IJobSystem.h"

#include "Common/System/IInput.h"

#include "Common/Memory/IMemory.h"

//----------------------------------------------------------
// Renderer relative
//----------------------------------------------------------
#include "Common/Render/Renderer.h"

//----------------------------------------------------------
// System data
//----------------------------------------------------------
#include "Common/Platform/Window.h"
#include "Common/Platform/OsDevices.h"

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