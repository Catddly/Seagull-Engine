#include "StdAfx.h"
#include "Allocator.h"

#include "Defs/Defs.h"

#include "Math/MathBasic.h"
#include "Profile/Profile.h"
#include "Memory/Memory.h"

namespace SG
{

	void* DefaultAllocator::allocate(Size size) noexcept
	{
		void* ptr = mi_malloc(size);
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_ALLOC(ptr, size);
#endif
		return ptr;
	}

	void* DefaultAllocator::allocate(Size size, Size alignment, Size alignmentOffset, int flags /*= 0*/) noexcept
	{
		SG_NO_USE(flags);
		if ((alignmentOffset % alignment) == 0)
		{
			void* ptr = mi_malloc_aligned(size, alignment);
#if SG_ENABLE_MEMORY_PROFILE
			SG_PROFILE_ALLOC(ptr, MinValueAlignTo(static_cast<UInt32>(size), static_cast<UInt32>(alignment)));
#endif
			return ptr;
		}
		return nullptr;
	}

	void DefaultAllocator::deallocate(void* ptr, Size size) noexcept
	{
		SG_NO_USE(size);
#if SG_ENABLE_MEMORY_PROFILE
		SG_PROFILE_FREE(ptr);
#endif
		mi_free(ptr);
	}

	DefaultAllocator::DefaultAllocator(const char* pName)
#ifdef _DEBUG
		: mName(pName)
#endif
	{}

	DefaultAllocator::DefaultAllocator(const DefaultAllocator& x)
	{
		*this = x;
	}

	DefaultAllocator::DefaultAllocator(const DefaultAllocator& x, const char* pName)
#ifdef _DEBUG
		:mName(pName)
#endif
	{
		*this = x;
	}

	const char* DefaultAllocator::get_name() const
	{
#ifdef _DEBUG
		return mName;
#else
		return "DefaultAllocator";
#endif
	}

	void DefaultAllocator::set_name(const char* pName)
	{
#ifdef _DEBUG
		mName = pName;
#endif
	}

	SG_CORE_API inline bool operator==(const DefaultAllocator& a, const DefaultAllocator& b)
	{
		return a.get_name() == b.get_name();
	}

	SG_CORE_API inline bool operator!=(const DefaultAllocator& a, const DefaultAllocator& b)
	{
		return !(a == b);
	}

	SG_CORE_API SG::DefaultAllocator  gDefaultAllocator;
	SG_CORE_API SG::DefaultAllocator* gpDefaultAllocator = &gDefaultAllocator;

	SG_CORE_API DefaultAllocator* GetDefaultAllocator()
	{
		return gpDefaultAllocator;
	}

	SG_CORE_API DefaultAllocator* SetDefaultAllocator(DefaultAllocator* pAllocator)
	{
		SG::DefaultAllocator* const pPrevAllocator = gpDefaultAllocator;
		gpDefaultAllocator = pAllocator;
		return pPrevAllocator;
	}

}