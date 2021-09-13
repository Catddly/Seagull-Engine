#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

#include "Thread/IThread.h"
#include "RID.h"

#include <EASTL/type_traits.h>

namespace SG
{

	template <typename T, bool bAligned = true, bool bIsPOD = eastl::is_pod_v<T>>
	class ResourcePool
	{
	public:
		ResourcePool(Size poolMemory = 65536);
		~ResourcePool();

		RID Allocate();

		T* const GetResource(RID rid);
	private:
		Size            mArenaSize;
		void*           mpMemory;
	};

	template <typename T, bool bAligned, bool bIsPOD>
	Atomic32 SG::ResourcePool<T, bAligned, bIsPOD>::sCurrentID = 1;

	template <typename T, bool bAligned, bool bIsPOD>
	SG::ResourcePool<T, bAligned>::ResourcePool(Size poolMemory)
		:mArenaSize(poolMemory), mpMemory(nullptr)
	{
		if constexpr (bAligned)
			SG_COMPILE_ASSERT(alignof(T) % 8 == 0, "Resource is not aligned to 8!");

		mpMemory = Memory::Malloc(poolMemory);

		if (!mpMemory)
		{
			SG_LOG_ERROR("No enough space to allocate (%d) memory!", poolMemory);
			SG_ASSERT(false);
		}
	}

	template <typename T, bool bAligned, bool bIsPOD>
	SG::ResourcePool<T, bAligned>::~ResourcePool()
	{
		Memory::Free(mpMemory);
	}

	template <typename T, bool bAligned, bool bIsPOD>
	T* const SG::ResourcePool<T, bAligned, bIsPOD>::GetResource(RID rid)
	{

	}

	template <typename T, bool bAligned, bool bIsPOD>
	RID SG::ResourcePool<T, bAligned, bIsPOD>::Allocate()
	{

	}


}