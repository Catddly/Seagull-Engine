#ifndef VECTOR_H
#define VECTOR_H

#pragma once

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "Core/STL/type_traits.h"

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

		pointer       data() noexcept       { return mBegin; }
		const_pointer data() const noexcept { return mBegin; }

		bool empty() const noexcept { return size() == 0; }
		Size capacity() const noexcept { return mCapacity; }
		Diff size() const noexcept { return Diff(mEnd - mBegin); }
		void clear() noexcept;

		iterator begin() noexcept { return mBegin; }
		iterator end()   noexcept { return mEnd; }
		const_iterator cbegin() const noexcept { return mBegin; }
		const_iterator cend()   const noexcept { return mEnd; }

		bool	   operator==(const this_type& rhs) const;
		this_type  operator=(const this_type& value);
		this_type  operator=(const this_type&& value);
		ref        operator[](Size index);
		const_ref  operator[](Size index) const;
	protected:
		void double_reserve();
	private:
		pointer mBegin;
		pointer mEnd;
		Size    mCapacity;
	};

	// vector default capacity if 8
	template<typename T>
	SG_INLINE vector<T>::vector() noexcept
		:mCapacity(8)
	{
		mBegin = reinterpret_cast<pointer>(Calloc(8, sizeof(value_type)));
		mEnd = mBegin;
		SG_ASSERT(mBegin && "Failed to allocate memory!");
	}

	template<typename T>
	SG_INLINE vector<T>::vector(Size s) noexcept
	{
		mCapacity = s / 8 == 0 ? 8 : ((s / 8) + 1) * 8;
		mBegin = reinterpret_cast<pointer>(Calloc(mCapacity, sizeof(value_type)));
		mEnd = mBegin;
	}

	template<typename T>
	SG_INLINE vector<T>::~vector() noexcept
	{
		if (mBegin)
		{
			Free(mBegin);
			mBegin = nullptr;
			mEnd = nullptr;
		}
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const this_type& vec)
	{
		mCapacity = vec.mCapacity;
		mBegin = reinterpret_cast<pointer>(Calloc(mCapacity, sizeof(value_type)));
		mEnd = mBegin + vec.size();
		memcpy(mData, vec.mData, sizeof(value_type) * vec.size());
	}

	template<typename T>
	SG_INLINE vector<T>::vector(const this_type&& vec) noexcept
	{
		mCapacity = vec.mCapacity;
		mBegin = vec.mBegin;
		mEnd = vec.mEnd;
		vec.mBegin = nullptr;
		vec.mEnd = nullptr;
	}

	template <typename T>
	void vector<T>::clear() noexcept
	{
		Free(mBegin);
		mBegin = reinterpret_cast<T*>(Calloc(8, sizeof(T)));
		SG_ASSERT(mBegin && "Failed to allocate memory!");
		mEnd = mBegin;
		mCapacity = 8;
	}

	template<typename T>
	SG_INLINE void vector<T>::push_back(const T& value)
	{
		if (size() < mCapacity)
		{
			*(mEnd) = value;
		}
		else
		{
			double_reserve();
			*(mEnd) = value;
		}
		++mEnd;
	}

	template<typename T>
	SG_INLINE T vector<T>::pop_back()
	{
		if (empty())
			SG_ASSERT(false && "Can't pop_back a empty vector!");
		else
		{
			T temp = *(mEnd - 1);
			--mEnd;
			return temp;
		}
		return T();
	}

	template<typename T>
	template<typename... Args>
	SG_INLINE typename vector<T>::iterator 
	vector<T>::emplace_back(Args&&... args)
	{
		if (size() <= mCapacity)
		{
			*(mEnd) = value_type(SG::forward<Args>(args)...);
		}
		else
		{
			double_reserve();
			*(mEnd) = value_type(SG::forward<Args>(args)...);
		}
		++mEnd;
		return (mEnd - 1);
	}

	template<typename T>
	SG_INLINE T* vector<T>::insert(Size index, const T& value)
	{
		if (index < 0 || index >= size())
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else if (size() == mCapacity)
		{
			double_reserve();
		}
		for (Size i = size() - 1; i > index; i--) // 6 9 (2) 1
		{
			*(mBegin + i + 1) = *(mBegin + i);
		}
		*(mBegin + index + 1) = value;
		++mEnd;
		return (mBegin + index + 1);
	}

	template<typename T>
	SG_INLINE T* vector<T>::erase(Size index)
	{
		if (index < 0 || index >= size())
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else
		{
			for (Size i = index; i < size() - 1; i++)
			{
				*(mBegin + i) = *(mBegin + i + 1);
			}
		}
		--mEnd;
		return (mBegin + index);
	}

	template<typename T>
	SG_INLINE void vector<T>::double_reserve()
	{
		Diff s = size();
		T* ptr = reinterpret_cast<pointer>(Calloc(2 * mCapacity, sizeof(value_type)));
		SG_ASSERT(mBegin && "Failed to allocate memory!");
		memcpy(ptr, mBegin, sizeof(value_type) * s);
		Free(mBegin);
		mBegin = ptr;
		mEnd = mBegin + s;
		mCapacity *= 2;
	}

	template<typename T>
	SG_INLINE bool vector<T>::operator==(const this_type& rhs) const
	{
		if (size() != rhs.size())
			return false;
		else
		{
			for (Size i = 0; i < size(); i++)
			{
				if (mBegin[i] != rhs.mBegin[i])
					return false;
			}
		}
		return true;
	}

	template<typename T>
	SG_INLINE typename vector<T>::this_type
	vector<T>::operator=(const this_type& value)
	{
		if (this != &value)
		{
			mCapacity = value.mCapacity;
			mBegin = reinterpret_cast<pointer>(Calloc(mCapacity, sizeof(value_type)));
			mEnd = mBegin + value.size();
			memcpy(mData, value.mData, sizeof(value_type) * value.size());
		}
		return *this;
	}

	template<typename T>
	SG_INLINE typename vector<T>::this_type
	vector<T>::operator=(const this_type&& value)
	{
		mCapacity = vec.mCapacity;
		mBegin = vec.mBegin;
		mEnd = vec.mEnd;
		vec.mBegin = nullptr;
		vec.mEnd = nullptr;
		return *this;
	}

	template<typename T>
	SG_INLINE typename vector<T>::ref
	vector<T>::operator[](Size index)
	{
#ifdef _DEBUG
		if (index >= size() || index < 0)
			SG_ASSERT(false && "Index over the array border!");
		else
#endif
			return *(mBegin + index);
	}

	template<typename T>
	SG_INLINE typename vector<T>::const_ref
	vector<T>::operator[](Size index) const
	{
#ifdef _DEBUG
		if (index >= size() || index < 0)
			SG_ASSERT(false && "Index over the array border!");
		else
#endif
		return *(mBegin + index);
	}

}

#endif // VECTOR_H