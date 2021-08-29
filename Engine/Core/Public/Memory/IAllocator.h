#ifndef IALLOCATOR_H
#define IALLOCATOR_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

namespace SG
{

	// if seagull should implement a memory_resource for abstraction
	// of memory?

	//! @Interface
	//! Abstraction for memory allocation
	interface SG_CORE_API IAllocator
	{
		IAllocator() = default;
		IAllocator(const IAllocator& x);
		virtual ~IAllocator() = default;

		virtual IAllocator& operator=(const IAllocator& x) { return *this; }

		virtual void* allocate(Size size) noexcept = 0;
		virtual void* allocate(Size n, Size alignment, Size alignmentOffset, int flags = 0) noexcept = 0;

		virtual void  deallocate(void* ptr, Size size) noexcept = 0;
	};

}

#endif // IALLOCATOR_H