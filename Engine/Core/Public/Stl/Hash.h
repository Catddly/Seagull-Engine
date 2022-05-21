#pragma once

#include "Base/BasicTypes.h"
#include "Defs/Defs.h"

#include <eastl/functional.h>

namespace SG
{

	// the output hash value is the seed!
	template <class T, class... Ts>
	void HashTypes(Size& seed, const T& value, const Ts&... otherValues)
	{
		seed ^= eastl::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(HashTypes(seed, otherValue), ...); // expand the rest types
	}

	// FNV-1 hash for memory
	SG_INLINE UInt64 HashMemory64(const UInt64* address, UInt64 prevHash = 14695981039346656037u)
	{
		UInt64 hash = prevHash;
		auto* perByteAddr = reinterpret_cast<const UInt8*>(address);
		for (UInt32 i = 0; i < 8; ++i) // for each byte
		{
			hash = hash * 1099511628211u;
			hash = hash ^ *(perByteAddr + i);
		}
		return hash;
	}

	SG_INLINE UInt64 HashMemory32(const UInt32* address, UInt64 prevHash = 2166136261u)
	{
		UInt64 hash = prevHash;
		auto* perByteAddr = reinterpret_cast<const UInt8*>(address);
		for (UInt32 i = 0; i < 4; ++i) // for each byte
		{
			hash = hash * 16777619u;
			hash = hash ^ *(perByteAddr + i);
		}
		return hash;
	}

	SG_INLINE UInt64 HashMemory32Array(const UInt32* arrayOfAddress, UInt32 count, UInt64 prevHash = 2166136261u)
	{
		UInt64 hash = HashMemory32(&arrayOfAddress[0], prevHash);
		for (UInt32 i = 1; i < count; ++i)
			hash = HashMemory32(&arrayOfAddress[i], hash);
		return hash;
	}

}