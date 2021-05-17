#pragma once

#include "Common/Memory/IAllocator.h"

namespace SG
{

	//! Seagull's default allocator using mimalloc lib
	class CDefaultAllocator final : public IAllocator
	{
	public:
		CDefaultAllocator(const CDefaultAllocator& x) = default;
		~CDefaultAllocator() {}

		virtual CDefaultAllocator& operator=(const CDefaultAllocator& x) { return *this; }

		virtual void* allocate(Size size) noexcept override;
		virtual void* allocate(Size size, Size alignment, Size alignmentOffset, int flags = 0) noexcept override;

		virtual void  deallocate(void* ptr, Size size) noexcept override;
	};

}