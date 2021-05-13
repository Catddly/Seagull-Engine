#pragma once

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

#include "Common/System/Memory/IMemory.h"
#include "Core/Memory/Memory.h"

namespace SG
{

	//! Seagull implemented tiny vector for stl
	template <typename T>
	class vector
	{
	public:
		typedef T  value_type;
		typedef T& ref_type;
		typedef T* iterator_type;
		typedef const T* const_iterator_type;
		typedef vector<T> type;

		vector();
		vector(const type&);
		vector(const type&&);
		~vector();

		vector(Size s);

		void       push_back(const T& value);
		value_type pop_back();
		void       emplace_back(T&& value);

		bool       empty() const { return mSize == 0; }
		Size       capacity() const { return mCapacity; }
		Size       size() const { return mSize; }

		iterator_type begin() { return mData; }
		iterator_type end()   { return mData + mSize; }
		const_iterator_type cbegin() const { return reinterpret_cast<const_iterator_type>(mData); }
		const_iterator_type cend() const { return reinterpret_cast<const_iterator_type>(mData + mSize); }

		bool	   operator==(const type& rhs) const;
		value_type operator=(const type& value);
		value_type operator=(const type&& value);
		ref_type   operator[](Size index);
	protected:
		void double_reserve();
	private:
		Size mSize;
		Size mCapacity;
		iterator_type mData;
	};

	// vector default capacity if 8
	template<typename T>
	SG_INLINE vector<T>::vector()
		:mCapacity(8), mSize(0)
	{
		mData = reinterpret_cast<T*>(Calloc(8, sizeof(T)));
		SG_ASSERT(mData && "Failed to allocate memory!");
	}

	template<typename T>
	SG_INLINE vector<T>::vector(Size s)
	{
		mCapacity = size > 8 ? size : 8;
		mSize = 0;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
	}

	template<typename T>
	SG_INLINE vector<T>::~vector()
	{
		if (mData) Free(mData);
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const type& vec)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
		memcpy(mData, vec.mData, sizeof(T) * mSize);
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const type&& vec)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = vec.mData;
		vec.mData = nullptr;
	}

	template<typename T>
	SG_INLINE void vector<T>::push_back(const T& value)
	{
		if (mSize < mCapacity)
		{
			mData[mSize] = value;
			++mSize;
		}
		else
		{
			double_reserve();
			mData[mSize] = value;
			++mSize;
		}
	}

	template<typename T>
	SG_INLINE T vector<T>::pop_back()
	{
		if (empty())
			SG_ASSERT(false && "Can't pop_back a empty vector!");
		else
		{
			T temp = mData[mSize - 1];
			--mSize;
			return temp;
		}
		return T();
	}

	template<typename T>
	SG_INLINE void vector<T>::emplace_back(T&& value)
	{
		if (mSize <= mCapacity)
		{
			mData[mSize] = std::move(value);
			++mSize;
		}
		else
		{
			double_reserve();
			mData[mSize] = std::move(value);
			++mSize;
		}
	}

	template<typename T>
	SG_INLINE void vector<T>::double_reserve()
	{
		T* ptr = reinterpret_cast<T*>(Calloc(2 * mCapacity, sizeof(T)));
		SG_ASSERT(mData && "Failed to allocate memory!");
		memcpy(ptr, mData, sizeof(T) * mSize);
		Free(mData);
		mData = ptr;
		mCapacity *= 2;
	}

	template<typename T>
	SG_INLINE bool vector<T>::operator==(const type& rhs) const
	{
		if (mSize != rhs.mSize)
			return false;
		else
		{
			for (Size i = 0; i < mSize; i++)
			{
				if (mData[i] != rhs.mData[i])
					return false;
			}
		}
		return true;
	}

	template<typename T>
	SG_INLINE T vector<T>::operator=(const type& value)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
		memcpy(mData, vec.mData, sizeof(T) * mSize);
		return *this;
	}

	template<typename T>
	SG_INLINE T vector<T>::operator=(const type&& value)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = vec.mData;
		vec.mData = nullptr;
		return *this;
	}

	template<typename T>
	SG_INLINE T& vector<T>::operator[](Size index)
	{
#ifdef _DEBUG
		if (index >= mSize)
			SG_ASSERT(false && "Index over the array border!");
		else
#endif
			return *(mData + index);
	}

}