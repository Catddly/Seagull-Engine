#ifndef IALLOCATOR_H
#define IALLOCATOR_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Base/BasicTypes.h"

namespace SG
{

	// if seagull should implement a memory_resource for abstraction
	// of memory?

	//! @Interface
	//! Abstraction for memory allocation
	class IAllocator
	{
		explicit IAllocator(const char* name = "seagull");
		IAllocator(const IAllocator& x);
		IAllocator(const IAllocator& x, const char* name);
		virtual ~IAllocator() = default;

		virtual IAllocator& operator=(const IAllocator& x) = 0;

		virtual void* allocate(Size size) noexcept = 0;
		virtual void* allocate(Size n, Size alignment, Size alignmentOffset, int flags = 0) noexcept = 0;

		virtual void  deallocate(void* ptr, Size size) noexcept = 0;
	};

}

#endif // IALLOCATOR_H