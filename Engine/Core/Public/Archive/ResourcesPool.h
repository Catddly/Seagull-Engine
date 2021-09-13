#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

#include "Thread/IThread.h"
#include "RID.h"
#include "Memory/IMemory.h"

#include <EASTL/type_traits.h>

namespace SG
{

	template <typename T, bool bThreadSafe = false, bool bIsPOD = eastl::is_pod_v<T>>
	class ResourcePool
	{
	public:
		ResourcePool(Size chunkSize = 4096);
		~ResourcePool();

		RID Allocate();
		void Free(const RID& rid);

		T* const GetResource(const RID& rid);
	private:
		struct Validation
		{
			UInt16 bUsed : 1;
			UInt16 bInitialized : 1;
		};
	private:
		UInt32 mNumChunks;
		UInt32 mNumMaxElementsPerChunk;
		UInt32 mCurrNumElements;

		T**          mppChunks    = nullptr;
		Validation*  mppValidator = nullptr;
	};

	template<typename T, bool bThreadSafe, bool bIsPOD>
	SG_INLINE ResourcePool<T, bThreadSafe, bIsPOD>::ResourcePool(Size chunkSize)
		:mNumMaxElementsPerChunk(chunkSize / sizeof(T)), mNumChunks(1), mCurrNumElements(0)
	{
		// TODO: replace 64 to GetCacheLineSize()
		SG_COMPILE_ASSERT(alignof(T) % 64 == 0, "Alignment of T is not align to cache line!");
	
		// pre-allocate memory pool
		mppChunks    = (T**)Memory::Malloc(sizeof(T*));
		mppChunks[0] = (T*)Memory::Calloc(mNumMaxElementsPerChunk, sizeof(T));
		mppValidator = (Validation*)Memory::Calloc(mNumMaxElementsPerChunk, sizeof(Validation));
	
		memset(mppValidator, 0, sizeof(Validation) * mNumMaxElementsPerChunk);
	}

	template<typename T, bool bThreadSafe, bool bIsPOD>
	SG_INLINE ResourcePool<T, bThreadSafe, bIsPOD>::~ResourcePool()
	{
		if (!bIsPOD)
		{
			for (UInt32 i = 0; i < mNumChunks * mNumMaxElementsPerChunk; i++)
			{
				UInt32 chunkIdx = i / mNumMaxElementsPerChunk;
				UInt32 lineIdx  = i % mNumMaxElementsPerChunk;

				if (mppValidator[i].bInitialized && mppValidator[i].bUsed)
					mppChunks[chunkIdx][lineIdx].~T();
			}
		}

		for (UInt32 i = 0; i < mNumChunks; i++)
			Memory::Free(mppChunks[i]);
		Memory::Free(mppValidator);
	}

	template<typename T, bool bThreadSafe, bool bIsPOD>
	SG_INLINE RID ResourcePool<T, bThreadSafe, bIsPOD>::Allocate()
	{
		RID rid;
		int chunkIdx = -1;
		int elementIdx = -1;
		for (UInt32 i = 0; i < mNumChunks * mNumMaxElementsPerChunk; i++)
		{
			if (!mppValidator[i].bUsed)
			{
				rid.mID = i;
				mppValidator[i].bUsed = true;
				rid.mID &= RID::VALID_MASK;
				chunkIdx   = i / mNumMaxElementsPerChunk;
				elementIdx = i % mNumMaxElementsPerChunk;

				if (bIsPOD)
				{
					memset(mppChunks[chunkIdx][elementIdx], 0, sizeof(T));
					mppValidator[i].bInitialized = true;
					rid.mID &= RID::INITIALIZED_MASK;
				}
				else
				{
					// TODO: add forwarding
				}

				break;
			}
		}

		return eastl::move(rid);
	}

	template<typename T, bool bThreadSafe, bool bIsPOD>
	SG_INLINE void ResourcePool<T, bThreadSafe, bIsPOD>::Free(const RID& rid)
	{
		SG_ASSERT(rid.mID < mNumMaxElementsPerChunk * mNumChunks);

		UInt32 id = rid.mID & RID::ID_MASK;
		bool bInitialized = rid.mID & RID::INITIALIZED_MASK;
		if (!bIsPOD && bInitialized)
		{
			UInt32 chunkIdx = id / mNumMaxElementsPerChunk;
			UInt32 elementIdx = id % mNumMaxElementsPerChunk;
			mppChunks[chunkIdx][elementIdx].~T();
		}

		mppValidator[id].bUsed = false;
		mppValidator[id].bInitialized = false;
	}


}