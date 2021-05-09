#pragma once

#include <stdint.h>

namespace SG
{
	using Float32 = float;
	using Float64 = double;

	using Int8  = int8_t;
	using Int16 = int16_t;
	using Int32 = int32_t;
	using Int64 = int64_t;

	using UInt8  = uint8_t;
	using UInt16 = uint16_t;
	using UInt32 = uint32_t;
	using UInt64 = uint64_t;

	using Size = size_t;

#ifdef __cplusplus
#	ifndef SG_ENUM_CLASS_FLAG
#		define SG_ENUM_CLASS_FLAG(VALUE_TYPE, ENUM_TYPE)																		 \
		static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b)   { return (ENUM_TYPE)((VALUE_TYPE)(a) | (VALUE_TYPE)(b)); } \
		static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b)   { return (ENUM_TYPE)((VALUE_TYPE)(a) & (VALUE_TYPE)(b)); } \
		static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a | b); }                                    \
		static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a & b); }
#	endif
#endif
#define SG_HAS_ENUM_FLAG(VALUE, FLAG) static_cast<bool>((VALUE) & (FLAG))

}