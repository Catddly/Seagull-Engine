#pragma once

#include "Core/Config.h"
#include "Memory/IAllocator.h"

namespace SG
{

	//! Seagull's default allocator using mimalloc lib
	class SG_CORE_API DefaultAllocator final : public IAllocator
	{
	public:
		explicit DefaultAllocator(const char* pName = "SG_DefaultAllocator");
		DefaultAllocator(const DefaultAllocator& x);
		DefaultAllocator(const DefaultAllocator& x, const char* pName);
		~DefaultAllocator() {}

		virtual DefaultAllocator& operator=(const DefaultAllocator& x) { return *this; }

		virtual void* allocate(Size size) noexcept override;
		virtual void* allocate(Size size, Size alignment, Size alignmentOffset, int flags = 0) noexcept override;

		virtual void  deallocate(void* ptr, Size size) noexcept override;

		const char*   get_name() const;
		void          set_name(const char* pName);
	private:
#ifdef _DEBUG
		const char* mName;
#endif
	};

	SG_CORE_API SG_INLINE bool operator==(const DefaultAllocator& a, const DefaultAllocator& b)
	{
		return a.get_name() == b.get_name();
	}

	SG_CORE_API SG_INLINE bool operator!=(const DefaultAllocator& a, const DefaultAllocator& b)
	{
		return !(a == b);
	}

	SG_CORE_API DefaultAllocator* GetDefaultAllocator();
	SG_CORE_API DefaultAllocator* SetDefaultAllocator(DefaultAllocator* pAllocator);

}