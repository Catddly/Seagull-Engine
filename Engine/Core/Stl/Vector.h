#pragma once

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

#include "Common/System/Memory/IMemory.h"

namespace SG
{

	//! Seagull implemented tiny vector
	template <typename T>
	class vector
	{
		typedef vector<T> this_type;
	public:
		typedef T  value_type;
		typedef T& ref;
		typedef T* pointer;
		typedef T* iterator;
		typedef const T& const_ref;
		typedef const T* const_pointer;
		typedef const T* const_iterator;

		vector() noexcept;
		vector(const this_type&);
		vector(const this_type&&) noexcept;
		~vector() noexcept;

		vector(Size s) noexcept;

		void       push_back(const T& value);
		value_type pop_back();
		template<typename... Args>
		iterator   emplace_back(Args&&... args);

		pointer insert(Size index, const T& value);
		pointer erase(Size index);

		pointer       data() noexcept       { return mData; }
		const_pointer data() const noexcept { return mData; }

		bool empty() const noexcept { return mSize == 0; }
		Size capacity() const noexcept { return mCapacity; }
		Size size() const noexcept { return mSize; }
		void clear() noexcept;

		iterator begin() noexcept { return mData; }
		iterator end()   noexcept { return mData + mSize; }
		const_iterator cbegin() const noexcept { return mData; }
		const_iterator cend()   const noexcept { return mData + mSize; }

		bool	   operator==(const this_type& rhs) const;
		value_type operator=(const this_type& value);
		value_type operator=(const this_type&& value);
		ref        operator[](Size index);
		const_ref  operator[](Size index) const;
	protected:
		void double_reserve();
	private:
		Size mSize;
		Size mCapacity;
		pointer mData;
	};

	// vector default capacity if 8
	template<typename T>
	SG_INLINE vector<T>::vector() noexcept
		:mCapacity(8), mSize(0)
	{
		mData = reinterpret_cast<T*>(Calloc(8, sizeof(T)));
		SG_ASSERT(mData && "Failed to allocate memory!");
	}

	template<typename T>
	SG_INLINE vector<T>::vector(Size s) noexcept
	{
		mCapacity = size > 8 ? size : 8;
		mSize = 0;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
	}

	template<typename T>
	SG_INLINE vector<T>::~vector() noexcept
	{
		if (mData)
		{
			Free(mData);
			mData = nullptr;
		}
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const this_type& vec)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
		memcpy(mData, vec.mData, sizeof(T) * mSize);
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const this_type&& vec) noexcept
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
	template<typename... Args>
	SG_INLINE typename vector<T>::iterator 
	vector<T>::emplace_back(Args&&... args)
	{
		if (mSize <= mCapacity)
		{
			mData[mSize] = T(std::forward<Args>(args)...);
			++mSize;
		}
		else
		{
			double_reserve();
			mData[mSize] = T(std::forward<Args>(args)...);
			++mSize;
		}
		return mData + mSize - 1;
	}

	template<typename T>
	SG_INLINE T* vector<T>::insert(Size index, const T& value)
	{
		if (index < 0 || index >= mSize)
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else if (mSize == mCapacity)
		{
			double_reserve();
		}
		for (Size i = mSize - 1; i > index; i--)
		{
			mData[i + 1] = mData[i];
		}
		mData[index + 1] = value;
		return (mData + index + 1);
	}

	template<typename T>
	SG_INLINE T* vector<T>::erase(Size index)
	{
		if (index < 0 || index >= mSize)
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else
		{
			for (Size i = index; i < mSize - 1; i++)
			{
				mData[i] = mData[i + 1];
			}
		}
		return (mData + index);
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
	SG_INLINE bool vector<T>::operator==(const this_type& rhs) const
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
	SG_INLINE T vector<T>::operator=(const this_type& value)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = reinterpret_cast<T*>(Calloc(mCapacity, sizeof(T)));
		memcpy(mData, vec.mData, sizeof(T) * mSize);
		return *this;
	}

	template<typename T>
	SG_INLINE T vector<T>::operator=(const this_type&& value)
	{
		mCapacity = vec.mCapacity;
		mSize = vec.mSize;
		mData = vec.mData;
		vec.mData = nullptr;
		return *this;
	}

	template<typename T>
	SG_INLINE typename vector<T>::ref
	vector<T>::operator[](Size index)
	{
#ifdef _DEBUG
		if (index >= mSize)
			SG_ASSERT(false && "Index over the array border!");
		else
#endif
			return *(mData + index);
	}

	template<typename T>
	SG_INLINE typename vector<T>::const_ref
	vector<T>::operator[](Size index) const
	{
		return *(mData + index);
	}

}