#include "StdAfx.h"
#include "Allocator.h"

#include "Common/Core/Defs.h"
#include "Common/Memory/IMemory.h"

#include <EASTL/allocator.h>

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

	CDefaultAllocator::CDefaultAllocator(const char* pName)
		: mName(pName)
	{}

	CDefaultAllocator::CDefaultAllocator(const CDefaultAllocator& x)
	{
		*this = x;
	}

	CDefaultAllocator::CDefaultAllocator(const CDefaultAllocator& x, const char* pName)
		:mName(pName)
	{
		*this = x;
	}

	const char* CDefaultAllocator::get_name() const
	{
		return mName;
	}

	void CDefaultAllocator::set_name(const char* pName)
	{
		mName = pName;
	}

	SG_COMMON_API inline bool operator==(const CDefaultAllocator& a, const CDefaultAllocator& b)
	{
		return a.get_name() == b.get_name();
	}

	SG_COMMON_API inline bool operator!=(const CDefaultAllocator& a, const CDefaultAllocator& b)
	{
		return !(a == b);
	}

	SG_COMMON_API SG::CDefaultAllocator  gDefaultAllocator;
	SG_COMMON_API SG::CDefaultAllocator* gpDefaultAllocator = &gDefaultAllocator;

	SG_COMMON_API CDefaultAllocator* GetDefaultAllocator()
	{
		return gpDefaultAllocator;
	}

	SG_COMMON_API CDefaultAllocator* SetDefaultAllocator(CDefaultAllocator* pAllocator)
	{
		SG::CDefaultAllocator* const pPrevAllocator = gpDefaultAllocator;
		gpDefaultAllocator = pAllocator;
		return pPrevAllocator;
	}

}