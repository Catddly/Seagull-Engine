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
	template <class T>
	UInt64 HashMemory64(const T* address)
	{
		UInt64 hash = 14695981039346656037;
		auto* perByteAddr = reinterpret_cast<UInt8*>(address);
		for (UInt32 i = 0; i < 8; ++i) // for each byte
		{
			hash = hash * 1099511628211;
			hash = hash ^ *(perByteAddr + i);
		}
		return hash;
	}

	template <class T>
	UInt64 HashMemory32(const T* address)
	{
		UInt64 hash = 2166136261;
		auto* perByteAddr = reinterpret_cast<UInt8*>(address);
		for (UInt32 i = 0; i < 4; ++i) // for each byte
		{
			hash = hash * 16777619;
			hash = hash ^ *(perByteAddr + i);
		}
		return hash;
	}

}