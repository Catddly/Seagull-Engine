#pragma once

#include <stdint.h>

#if INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
#	define SPTR_SIZE 8
#else INTPTR_MAX == 0x7FFFFFFF
#	define SPTR_SIZE 4
#endif

