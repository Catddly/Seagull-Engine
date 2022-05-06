#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "eastl/queue.h"

namespace SG
{

	enum class EIDAllocatorType
	{
		eRestored = 0, //! id can be reuse by allocator.
		eOneWay,       //! id can NOT be reuse by allocator, the object allocated first must have smaller id than the later.
	};
	
	//! This is an sequential id allocator.
	//! Implementation: (array with hole)
	template <typename IDType = UInt32, EIDAllocatorType AllcatorType = EIDAllocatorType::eRestored>
	class IDAllocator;

	// template specialization for Restored IDAllocator
	template <typename IDType>
	class IDAllocator<IDType, EIDAllocatorType::eRestored>
	{
	private:
		using TID = IDType;
		using ThisType = IDAllocator<IDType, EIDAllocatorType::eRestored>;
	public:
		static constexpr TID INVALID_ID = TID(-1);

		bool IsValid(TID id);

		TID  Allocate();
		void Restore(TID id);
	private:
		TID mCurrentAvailableId = TID(0);
		eastl::queue<TID> mRestoredId;
	};

	template <typename IDType>
	bool IDAllocator<IDType, EIDAllocatorType::eRestored>::IsValid(TID id)
	{
		return id != INVALID_ID;
	}

	template <typename IDType>
	typename IDAllocator<IDType, EIDAllocatorType::eRestored>::TID IDAllocator<IDType, EIDAllocatorType::eRestored>::Allocate()
	{
		SG_ASSERT(IsValid(mCurrentAvailableId + 1));
		if (mRestoredId.empty())
			return mCurrentAvailableId++;
		else
		{
			TID id = mRestoredId.front();
			mRestoredId.pop();
			return id;
		}
	}

	template <typename IDType>
	void IDAllocator<IDType, EIDAllocatorType::eRestored>::Restore(TID id)
	{
		SG_ASSERT(IsValid(id));
		mRestoredId.push(id);
	}

	// template specialization for OneWay IDAllocator
	template <typename IDType>
	class IDAllocator<IDType, EIDAllocatorType::eOneWay>
	{
	private:
		using TID = IDType;
		using ThisType = IDAllocator<IDType, EIDAllocatorType::eOneWay>;
	public:
		static constexpr TID INVALID_ID = TID(-1);

		bool IsValid(TID id);

		TID  Allocate();
		void Restore(TID id);
	private:
		TID mCurrentAvailableId = TID(0);
	};

	template <typename IDType>
	bool IDAllocator<IDType, EIDAllocatorType::eOneWay>::IsValid(TID id)
	{
		return id != INVALID_ID;
	}

	template <typename IDType>
	typename IDAllocator<IDType, EIDAllocatorType::eOneWay>::TID IDAllocator<IDType, EIDAllocatorType::eOneWay>::Allocate()
	{
		SG_ASSERT(IsValid(mCurrentAvailableId + 1));
		return mCurrentAvailableId++;
	}

	template <typename IDType>
	void IDAllocator<IDType, EIDAllocatorType::eOneWay>::Restore(TID id)
	{
		SG_ASSERT(IsValid(id));
	}

}