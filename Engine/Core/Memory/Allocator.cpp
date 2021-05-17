#include "StdAfx.h"
#include "Allocator.h"

#include "Common/Core/Defs.h"
#include "Common/Memory/IMemory.h"

namespace SG
{

	void* CDefaultAllocator::allocate(Size size) noexcept
	{
		return Malloc(size);
	}

	void* CDefaultAllocator::allocate(Size size, Size alignment, Size alignmentOffset, int flags /*= 0*/) noexcept
	{
		SG_NO_USE(flags);
		if ((alignmentOffset % alignment) == 0)
			return MallocAlign(size, alignment);
		return nullptr;
	}

	void CDefaultAllocator::deallocate(void* ptr, Size size) noexcept
	{
		SG_NO_USE(size);
		Free(ptr);
	}

}