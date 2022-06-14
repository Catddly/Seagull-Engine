#pragma once

#include "Defs/Defs.h"

#include <stdint.h>
#include <numeric>

namespace SG
{

	typedef void (*FileTraverseFunc)(const char* filename);

	using Float32 = float;
	using Float64 = double;
	using LFloat  = long double;

	using Int8  = int8_t;
	using Int16 = int16_t;
	using Int32 = int32_t;
	using Int64 = int64_t;

	using UInt8  = uint8_t;
	using UInt16 = uint16_t;
	using UInt32 = uint32_t;
	using UInt64 = uint64_t;

	using Char  = char;
	using UChar = unsigned char;
	using SChar = signed char;
	using WChar = wchar_t;

	using Char8  = char;
	using Char16 = char16_t;
	using Char32 = char32_t;

	using Size = size_t;
	using Diff = long long;
	using IntPtr = intptr_t;
	using UIntPtr = uintptr_t;

#if SG_CPP_VERSION_17
	using Byte = std::byte;
#else
	using Byte = unsigned char;
#endif

#define SG_FLOAT_EPSILON  std::numeric_limits<float>::epsilon()
#define SG_DOUBLE_EPSILON std::numeric_limits<double>::epsilon()

#define SG_MIN_FLOAT_VALUE std::numeric_limits<float>::lowest()
#define SG_MAX_FLOAT_VALUE std::numeric_limits<float>::max()

#ifdef __cplusplus
#	define SG_ENUM_CLASS_FLAG(VALUE_TYPE, ENUM_TYPE)																	   \
	constexpr ENUM_TYPE  operator| (ENUM_TYPE a, VALUE_TYPE b)  { return (ENUM_TYPE)((VALUE_TYPE)(a) | b); }  \
	constexpr ENUM_TYPE  operator| (ENUM_TYPE a, ENUM_TYPE b)  { return (ENUM_TYPE)((VALUE_TYPE)(a) | (VALUE_TYPE)(b)); }  \
	constexpr ENUM_TYPE  operator& (ENUM_TYPE a, VALUE_TYPE b)  { return (ENUM_TYPE)((VALUE_TYPE)(a) & b); }  \
	constexpr ENUM_TYPE  operator& (ENUM_TYPE a, ENUM_TYPE b)  { return (ENUM_TYPE)((VALUE_TYPE)(a) & (VALUE_TYPE)(b)); }  \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& a, VALUE_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) | b); return a; }  \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& a, ENUM_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) | (VALUE_TYPE)(b)); return a; }   \
	constexpr ENUM_TYPE& operator&=(ENUM_TYPE& a, VALUE_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) & b); return a; }  \
	constexpr ENUM_TYPE& operator&=(ENUM_TYPE& a, ENUM_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) & (VALUE_TYPE)(b)); return a; } 
#endif
#define SG_HAS_ENUM_FLAG(VALUE, FLAG) static_cast<bool>((VALUE) & (FLAG))

}