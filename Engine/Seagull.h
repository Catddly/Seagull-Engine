#if defined(SG_USE_DLL)
#	define SG_COMMON_API   __declspec(dllimport)
#	define SG_ENGINE_API __declspec(dllimport)
#else
#	define SG_COMMON_API
#	define SG_ENGINE_API
#endif


#include "Common/System/IApp.h"

//----------------------------------------------------------
// EntryPoint
//----------------------------------------------------------
#include "Common/EntryPoint.h"