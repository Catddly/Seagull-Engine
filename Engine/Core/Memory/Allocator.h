#pragma once

#include "Common/Config.h"
#include "Common/Memory/IAllocator.h"

namespace SG
{

	//! Seagull's default allocator using mimalloc lib
	class SG_COMMON_API CDefaultAllocator final : public IAllocator
	{
	public:
		explicit CDefaultAllocator(const char* pName = "SG_DefaultAllocator");
		CDefaultAllocator(const CDefaultAllocator& x);
		CDefaultAllocator(const CDefaultAllocator& x, const char* pName);
		~CDefaultAllocator() {}

		virtual CDefaultAllocator& operator=(const CDefaultAllocator& x) { return *this; }

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

	SG_COMMON_API bool operator==(const CDefaultAllocator& a, const CDefaultAllocator& b);
	SG_COMMON_API inline bool operator!=(const CDefaultAllocator& a, const CDefaultAllocator& b);

	SG_COMMON_API CDefaultAllocator* GetDefaultAllocator();
	SG_COMMON_API CDefaultAllocator* SetDefaultAllocator(CDefaultAllocator* pAllocator);

}