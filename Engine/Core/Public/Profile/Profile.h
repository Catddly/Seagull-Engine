#pragma once

#include "Defs/Defs.h"

#if SG_ENABLE_PROFILE
#	define TRACY_ENABLE
#	include "tracy/Tracy.hpp"
#endif

namespace SG
{

#if SG_ENABLE_PROFILE
#	define SG_PROFILE_FUNCTION() ZoneScoped
#	define SG_PROFILE_SCOPE(NAME) ZoneScopedN(NAME)
#	define SG_PROFILE_FRAME_MARK() FrameMark
#	define SG_PROFILE_ALLOC(PTR, SIZE) TracyAlloc(PTR, SIZE)
#	define SG_PROFILE_FREE(PTR) TracyFree(PTR)
#else
#	define SG_PROFILE_FUNCTION()
#	define SG_PROFILE_SCOPE(NAME)
#	define SG_PROFILE_FRAME_MARK()
#	define SG_PROFILE_ALLOC(PTR, SIZE)
#	define SG_PROFILE_FREE(PTR, SIZE)
#endif

}