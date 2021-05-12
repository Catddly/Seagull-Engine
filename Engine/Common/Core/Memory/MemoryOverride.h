#pragma once

#ifdef USE_SEAGULL_MEMORY
#	ifdef USE_EXTERN_MIMALLOC
#		ifndef sg_new
#		define sg_new mimalloc::new
#		endif

#		ifndef sg_delete
#		define sg_delete mimalloc::delete
#		endif
#	endif
#endif