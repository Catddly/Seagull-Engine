#pragma once

#include "Defs/Defs.h"
#include "Memory/Memory.h"

#include "eastl/type_traits.h"

namespace SG
{

	// forward decoration
	template <typename T>
	class Handle;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ReadOnlyHandle
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	class ReadOnlyHandle final
	{
	private:
		static_assert(eastl::is_pointer_v<T>, "T is not a pointer!");
		using DataType = T;
		using ThisType = ReadOnlyHandle<T>;
	public:
		ReadOnlyHandle();
		~ReadOnlyHandle();
		SG_CLASS_NO_COPY_ASSIGNABLE(ReadOnlyHandle);

		ReadOnlyHandle(ThisType&&) noexcept;
		ThisType& operator=(ThisType&&) noexcept;

		bool     IsValid() const noexcept;
		DataType GetData() const noexcept;
	private:
		friend class Handle<T>;

		ReadOnlyHandle(DataType* pData, DataType* pFallbackData, int* pVersionNumber, int* pRefCount);
	private:
		DataType* mpData;
		DataType* mpFallBackData;
		int* mpVersionNumber;
		int* mpRefCount;
		int  mMyVersionNumber;
	};

	template <typename T>
	ReadOnlyHandle<T>::ReadOnlyHandle(ThisType&& other) noexcept
	{
		this->mpData = eastl::move(other.mpData);
		this->mpFallBackData = eastl::move(other.mpFallBackData);
		this->mpVersionNumber = eastl::move(other.mpVersionNumber);
		this->mpRefCount = eastl::move(other.mpRefCount);

		other.mpData = nullptr;
		other.mpFallBackData = nullptr;
		other.mpRefCount = nullptr;
		other.mpVersionNumber = nullptr;
		other.mMyVersionNumber = -1;

		this->mMyVersionNumber = 0;
		//SG_LOG_DEBUG("ReadOnlyHandle move ctor!");
	}

	template <typename T>
	typename ReadOnlyHandle<T>::ThisType& ReadOnlyHandle<T>::operator=(ThisType&& other) noexcept
	{
		if (&other != this)
		{
			if (mpRefCount)
			{
				(*mpRefCount) -= 1;
				if (*mpRefCount == 0)
				{
					Delete(mpRefCount);
					Delete(mpVersionNumber);
					Delete(mpData);
					Delete(mpFallBackData);
					//SG_LOG_DEBUG("Move Self destroyed!");
				}
			}

			this->mpData = eastl::move(other.mpData);
			this->mpFallBackData = eastl::move(other.mpFallBackData);
			this->mpVersionNumber = eastl::move(other.mpVersionNumber);
			this->mpRefCount = eastl::move(other.mpRefCount);

			other.mpData = nullptr;
			other.mpFallBackData = nullptr;
			other.mpRefCount = nullptr;
			other.mpVersionNumber = nullptr;
			other.mMyVersionNumber = -1;

			this->mMyVersionNumber = 0;
			//SG_LOG_DEBUG("Handle move assignment operator!");
		}
		return *this;
	}

	template <typename T>
	ReadOnlyHandle<T>::ReadOnlyHandle()
		:mpData(nullptr), mpFallBackData(nullptr), mpVersionNumber(nullptr), mpRefCount(nullptr),
		mMyVersionNumber(-1)
	{
	}

	template <typename T>
	bool ReadOnlyHandle<T>::IsValid() const noexcept
	{
		return mMyVersionNumber == *mpVersionNumber;
	}

	template <typename T>
	typename ReadOnlyHandle<T>::DataType ReadOnlyHandle<T>::GetData() const noexcept
	{
		if (!IsValid())
			return *mpFallBackData;
		else
			return *mpData;
	}

	template <typename T>
	ReadOnlyHandle<T>::ReadOnlyHandle(DataType* pData, DataType* pFallbackData,
		int* pVersionNumber, int* pRefCount)
		:mpData(pData), mpFallBackData(pFallbackData), mpVersionNumber(pVersionNumber), mpRefCount(pRefCount),
		mMyVersionNumber(0)
	{
		(*mpRefCount) += 1;
		//SG_LOG_DEBUG("ReadOnlyHandle created with ref count: %d", *mpRefCount);
	}

	template <typename T>
	ReadOnlyHandle<T>::~ReadOnlyHandle()
	{
		if (mpRefCount)
		{
			(*mpRefCount) -= 1;
			if (*mpRefCount == 0)
			{
				Delete(mpVersionNumber);
				Delete(mpRefCount);
				Delete(mpData);
				Delete(mpFallBackData);
				//SG_LOG_DEBUG("ReadOnlyHandle destroyed!");
			}
		}
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Handle
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! View of data, it will not destroy the data, please release the data manually.
	template <typename T>
	class Handle final
	{
	private:
		static_assert(eastl::is_pointer_v<T>, "T is not a pointer!");
		using DataType = T;
		using ThisType = Handle<T>;
	public:
		Handle();
		Handle(DataType pData, DataType pFallbackData);
		~Handle();

		ReadOnlyHandle<T> ReadOnly() const noexcept;

		Handle(const ThisType&) noexcept;
		Handle(ThisType&&) noexcept;
		ThisType& operator=(const ThisType& other) noexcept;
		ThisType& operator=(ThisType&&) noexcept;

		void Validate() noexcept;
		void Invalidate() noexcept;

		bool IsValid() const noexcept;
		
		void      SetData(DataType pData) noexcept;
		void      SetFallBackData(DataType pData) noexcept;
		DataType  GetData() const noexcept;
		DataType  GetFallBackData() const noexcept;
	private:
		DataType* mpData;
		DataType* mpFallBackData;
		int* mpVersionNumber;
		int* mpRefCount;
		int  mMyVersionNumber;
	};

	template <typename T>
	Handle<T>::Handle()
		:mMyVersionNumber(0)
	{
		mpVersionNumber = New(int, 0);
		mpRefCount = New(int, 0);
		mpData = New(DataType, nullptr);
		mpFallBackData = New(DataType, nullptr);
		SG_ASSERT(mpVersionNumber && mpRefCount && mpData && mpFallBackData);
		(*mpRefCount) += 1;
		//SG_LOG_DEBUG("Handle created with ref count: %d", *mpRefCount);
	}

	template <typename T>
	Handle<T>::Handle(DataType pData, DataType pFallbackData)
		:mMyVersionNumber(0)
	{
		mpVersionNumber = New(int, 0);
		mpRefCount = New(int, 0);
		mpData = New(DataType, nullptr);
		mpFallBackData = New(DataType, nullptr);
		SG_ASSERT(mpVersionNumber && mpRefCount && mpData && mpFallBackData);
		*mpData = pData;
		*mpFallBackData = pFallbackData;
		(*mpRefCount) += 1;
		//SG_LOG_DEBUG("Handle created with ref count: %d", *mpRefCount);
	}

	template <typename T>
	void Handle<T>::SetData(DataType pData) noexcept
	{
		*mpData = pData;
	}

	template <typename T>
	void Handle<T>::SetFallBackData(DataType pData) noexcept
	{
		*mpFallBackData = pData;
	}

	template <typename T>
	ReadOnlyHandle<T> Handle<T>::ReadOnly() const noexcept
	{
		return ReadOnlyHandle<T>(mpData, mpFallBackData, mpVersionNumber, mpRefCount);
	}

	template <typename T>
	typename Handle<T>::DataType Handle<T>::GetData() const noexcept
	{
		if (!IsValid())
			return *mpFallBackData;
		else
			return *mpData;
	}

	template <typename T>
	typename Handle<T>::DataType Handle<T>::GetFallBackData() const noexcept
	{
		return *mpFallBackData;
	}

	template <typename T>
	Handle<T>::~Handle()
	{
		if (mpRefCount)
		{
			(*mpRefCount) -= 1;
			if (*mpRefCount == 0)
			{
				Delete(mpVersionNumber);
				Delete(mpRefCount);
				Delete(mpData);
				Delete(mpFallBackData);
				//SG_LOG_DEBUG("Handle destroyed!");
			}
		}
	}

	template <typename T>
	Handle<T>::Handle(const ThisType& other) noexcept
	{
		this->mpData = other.mpData;
		this->mpFallBackData = other.mpFallBackData;
		this->mpVersionNumber = other.mpVersionNumber;
		this->mpRefCount = other.mpRefCount;
		this->mMyVersionNumber = 0;
		(*mpRefCount) += 1;
		//SG_LOG_DEBUG("Handle copy ctor!");
	}

	template <typename T>
	Handle<T>::Handle(ThisType&& other) noexcept
	{
		this->mpData = eastl::move(other.mpData);
		this->mpFallBackData = eastl::move(other.mpFallBackData);
		this->mpVersionNumber = eastl::move(other.mpVersionNumber);
		this->mpRefCount = eastl::move(other.mpRefCount);

		other.mpData = nullptr;
		other.mpFallBackData = nullptr;
		other.mpRefCount = nullptr;
		other.mpVersionNumber = nullptr;
		other.mMyVersionNumber = -1;

		this->mMyVersionNumber = 0;
		//SG_LOG_DEBUG("Handle move ctor!");
	}

	template <typename T>
	typename Handle<T>::ThisType& Handle<T>::operator=(const ThisType& other) noexcept
	{
		if (&other != this)
		{
			(*mpRefCount) -= 1;
			if (*mpRefCount == 0)
			{
				Delete(mpRefCount);
				Delete(mpVersionNumber);
				Delete(mpData);
				Delete(mpFallBackData);
				//SG_LOG_DEBUG("Copy Self destroyed!");
			}

			this->mpData = other.mpData;
			this->mpFallBackData = other.mpFallBackData;
			this->mpVersionNumber = other.mpVersionNumber;
			this->mpRefCount = other.mpRefCount;
			this->mMyVersionNumber = 0;
			(*mpRefCount) += 1;
			//SG_LOG_DEBUG("Handle copy assignment operator!");
		}
		return *this;
	}

	template <typename T>
	typename Handle<T>::ThisType& Handle<T>::operator=(ThisType&& other) noexcept
	{
		if (&other != this)
		{
			(*mpRefCount) -= 1;
			if (*mpRefCount == 0)
			{
				Delete(mpRefCount);
				Delete(mpVersionNumber);
				Delete(mpData);
				Delete(mpFallBackData);
				//SG_LOG_DEBUG("Move Self destroyed!");
			}

			this->mpData = eastl::move(other.mpData);
			this->mpFallBackData = eastl::move(other.mpFallBackData);
			this->mpVersionNumber = eastl::move(other.mpVersionNumber);
			this->mpRefCount = eastl::move(other.mpRefCount);

			other.mpData = nullptr;
			other.mpFallBackData = nullptr;
			other.mpRefCount = nullptr;
			other.mpVersionNumber = nullptr;
			other.mMyVersionNumber = -1;

			this->mMyVersionNumber = 0;
			//SG_LOG_DEBUG("Handle move assignment operator!");
		}
		return *this;
	}

	template <typename T>
	void Handle<T>::Invalidate() noexcept
	{
		(*mpVersionNumber) += 1;
	}

	template <typename T>
	void Handle<T>::Validate() noexcept
	{
		*mpVersionNumber = 0;
	}

	template <typename T>
	bool Handle<T>::IsValid() const noexcept
	{
		return mMyVersionNumber == *mpVersionNumber;
	}

}