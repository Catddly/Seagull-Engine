#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

//----------------------------------------------------------
// Standard template library
//----------------------------------------------------------
#include "Common/Stl/vector.h"
#include "Common/Stl/string.h"
#include "Common/Stl/string_view.h"
#include "Common/Stl/shared_ptr.h"

//----------------------------------------------------------
// Math library
//----------------------------------------------------------
#include "Common/Math/Vector.h"
#include "Common/Math/Matrix.h"

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
// Render relative
//----------------------------------------------------------
#include "Common/Render/Queue.h"
#include "Common/Render/RenderContext.h"
#include "Common/Render/Texture.h"
#include "Common/Render/Renderer.h"

//----------------------------------------------------------
// System data
//----------------------------------------------------------
#include "Common/Platform/Window.h"
#include "Common/Platform/OsDevices.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "EntryPoint.h"