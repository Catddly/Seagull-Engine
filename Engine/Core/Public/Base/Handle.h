#pragma once

#include "Memory/Memory.h"

#include "eastl/type_traits.h"

namespace SG
{

	template <typename T>
	class ReadOnlyHandle
	{
	public:

	private:
	};

	template <typename T>
	class Handle
	{
	private:
		using DataType = eastl::decay<T>::type;
		using ThisType = Handle<T>;
	public:
		Handle(DataType* pData, DataType* pFallbackData);
		~Handle();

		Handle(const ThisType&) = delete;
		Handle(ThisType&&) = delete;
		ThisType& operator(const ThisType&) = delete;
		ThisType& operator(ThisType&&) = delete;

		void Validate();
		void Invalidate();

		bool IsValid() const;
		
		DataType* GetData() const;
	private:
		DataType* mpData;
		DataType* mpFallBackData;
		int  mMyVersionNumber;
		int* mpVersionNumber;
		int* mpRefCount;
	};

	template <typename T>
	Handle<T>::Handle(DataType* pData, DataType* pFallbackData)
		:mpData(pData), mpFallBackData(pFallbackData), mMyVersionNumber(0)
	{
		SG_ASSERT(!pData);
		mpVersionNumber = Memory::New<int>(0);
		mpRefCount = Memory::New<int>(0);
		(*mpRefCount) += 1;
	}

	template <typename T>
	Handle<T>::~Handle()
	{
		(*mpRefCount) -= 1;
		if (*mpRefCount == 0)
		{
			Memory::Delete(mpVersionNumber);
			Memory::Delete(mpRefCount);
		}
	}

	template <typename T>
	void Handle<T>::Invalidate()
	{
		(*mpVersionNumber) += 1;
	}

	template <typename T>
	void Handle<T>::Validate()
	{
		mpVersionNumber = mMyVersionNumber;
	}

	template <typename T>
	bool Handle<T>::IsValid() const
	{
		return mMyVersionNumber == *mpVersionNumber;
	}

	template <typename T>
	Handle<T>::DataType* Handle<T>::GetData() const
	{
		if (!IsValid())
			return mpFallBackData;
		else
			return mpData;
	}

}