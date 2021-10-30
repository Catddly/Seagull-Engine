#include "StdAfx.h"
#include "Allocator.h"

#include "Defs/Defs.h"
#include "Memory/Memory.h"

namespace SG
{

	void* CDefaultAllocator::allocate(Size size) noexcept
	{
		return Memory::Malloc(size);
	}

	void* CDefaultAllocator::allocate(Size size, Size alignment, Size alignmentOffset, int flags /*= 0*/) noexcept
	{
		SG_NO_USE(flags);
		if ((alignmentOffset % alignment) == 0)
			return Memory::MallocAlign(size, alignment);
		return nullptr;
	}

	void CDefaultAllocator::deallocate(void* ptr, Size size) noexcept
	{
		SG_NO_USE(size);
		Memory::Free(ptr);
	}

	CDefaultAllocator::CDefaultAllocator(const char* pName)
#ifdef _DEBUG
		: mName(pName)
#endif
	{}

	CDefaultAllocator::CDefaultAllocator(const CDefaultAllocator& x)
	{
		*this = x;
	}

	CDefaultAllocator::CDefaultAllocator(const CDefaultAllocator& x, const char* pName)
#ifdef _DEBUG
		:mName(pName)
#endif
	{
		*this = x;
	}

	const char* CDefaultAllocator::get_name() const
	{
#ifdef _DEBUG
		return mName;
#else
		return "DefaultAllocator";
#endif
	}

	void CDefaultAllocator::set_name(const char* pName)
	{
#ifdef _DEBUG
		mName = pName;
#endif
	}

	SG_CORE_API inline bool operator==(const CDefaultAllocator& a, const CDefaultAllocator& b)
	{
		return a.get_name() == b.get_name();
	}

	SG_CORE_API inline bool operator!=(const CDefaultAllocator& a, const CDefaultAllocator& b)
	{
		return !(a == b);
	}

	SG_CORE_API SG::CDefaultAllocator  gDefaultAllocator;
	SG_CORE_API SG::CDefaultAllocator* gpDefaultAllocator = &gDefaultAllocator;

	SG_CORE_API CDefaultAllocator* GetDefaultAllocator()
	{
		return gpDefaultAllocator;
	}

	SG_CORE_API CDefaultAllocator* SetDefaultAllocator(CDefaultAllocator* pAllocator)
	{
		SG::CDefaultAllocator* const pPrevAllocator = gpDefaultAllocator;
		gpDefaultAllocator = pAllocator;
		return pPrevAllocator;
	}

}