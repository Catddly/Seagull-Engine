#ifndef VECTOR_H
#define VECTOR_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "Common/Base/IIterable.h"
#include "Core/STL/type_traits.h"
#include "Core/STL/internal/copy_and_move.h"

#include "Common/Memory/IMemory.h"

namespace SG
{

	//! Seagull implemented tiny vector(no allocator)
	template <typename T>
	class vector : public IIterable<T>
	{
		typedef vector<T> this_type;
	public:
		typedef T  value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef const T& const_reference;
		typedef const T* const_pointer;

		using IIterable::difference_type;
		using IIterable::iterator;
		using IIterable::const_iterator;
		using IIterable::reverse_iterator;
		using IIterable::const_reverse_iterator;

		vector() noexcept;
		vector(const this_type&);
		vector(this_type&&) noexcept;
		~vector() noexcept;

		vector(Size s) noexcept;

		void swap(this_type& vec);

		void       push_back(const T& value);
		value_type pop_back();
		template<typename... Args>
		iterator   emplace_back(Args&&... args);

		pointer insert(Size index, const T& value);
		pointer erase(Size index);

		pointer       data() noexcept       { return mBegin; }
		const_pointer data() const noexcept { return mBegin; }

		bool empty() const noexcept { return mBegin == mEnd; }
		Size capacity() const noexcept { return mCapacity; }
		Diff size() const noexcept { return Diff(mEnd - mBegin); }
		//! Just call the destructor of the value_type of the iterator, no memory freed, capacity remained
		void clear() noexcept;

		virtual iterator begin() noexcept override { return mBegin; }
		virtual const_iterator begin()  const noexcept override { return mBegin; }
		virtual const_iterator cbegin() const noexcept override { return mBegin; }

		virtual iterator end()   noexcept  override { return mEnd; }
		virtual const_iterator end()  const noexcept override { return mEnd; }
		virtual const_iterator cend() const noexcept override { return mEnd; }

		virtual reverse_iterator rbegin()  noexcept override { return reverse_iterator(mEnd); }
		virtual const_reverse_iterator rbegin()  const noexcept override { return const_reverse_iterator(mEnd); }
		virtual const_reverse_iterator crbegin() const noexcept override { return const_reverse_iterator(mEnd); }

		virtual reverse_iterator rend()  noexcept override { return reverse_iterator(mBegin); }
		virtual const_reverse_iterator rend()  const noexcept override { return const_reverse_iterator(mBegin); }
		virtual const_reverse_iterator crend() const noexcept override { return const_reverse_iterator(mBegin); }

		bool	   operator==(const this_type& rhs) const;
		this_type  operator=(const this_type& value);
		this_type  operator=(this_type&& value) noexcept;
		reference        operator[](Size index);
		const_reference  operator[](Size index) const;
	protected:
		void double_reserve();
	private:
		pointer mBegin;
		pointer mEnd;
		Size    mCapacity;
	};

	// vector default capacity if 8
	template<typename T>
	vector<T>::vector() noexcept
		:mCapacity(8)
	{
		mBegin = reinterpret_cast<pointer>(CallocAlign(8, sizeof(value_type), 8));
		mEnd = mBegin;
		SG_ASSERT(mBegin && "Failed to allocate memory!");
	}

	template<typename T>
	vector<T>::vector(Size s) noexcept
	{
		mCapacity = s / 8 == 0 ? 8 : ((s / 8) + 1) * 8;
		mBegin = reinterpret_cast<pointer>(CallocAlign(mCapacity, sizeof(value_type), 8));
		mEnd = mBegin;
	}

	template<typename T>
	vector<T>::~vector() noexcept
	{
		if (mBegin)
		{
			Free(mBegin);
			mBegin = nullptr;
			mEnd = mBegin;
			mCapacity = 0;
		}
	}

	template <typename T>
	SG::vector<T>::vector(const this_type& vec)
		:mBegin(nullptr), mEnd(nullptr), mCapacity(vec.mCapacity)
	{
		mBegin = reinterpret_cast<pointer>(Calloc(mCapacity, sizeof(value_type)));
		mEnd = SG::copy_ptr_uninitialzied(vec.mBegin, vec.mEnd, mBegin);
	}

	template<typename T>
	vector<T>::vector(this_type&& vec) noexcept
		:mBegin(nullptr), mEnd(nullptr), mCapacity(0)
	{
		swap(vec);
	}

	template <typename T>
	SG_INLINE void SG::vector<T>::swap(this_type& vec)
	{
		std::swap(mBegin, vec.mBegin);
		std::swap(mEnd, vec.mEnd);
		std::swap(mCapacity, vec.mCapacity);
	}

	template <typename T>
	SG_INLINE void vector<T>::clear() noexcept
	{
		Destruct(mBegin, mEnd);
		mEnd = mBegin;
	}

	template<typename T>
	SG_INLINE void vector<T>::push_back(const T& value)
	{
		if (Size(size()) < mCapacity)
		{
			*(mEnd) = value;
			++mEnd;
		}
		else
		{
			double_reserve();
			push_back(value);
		}
	}

	template<typename T>
	SG_INLINE T vector<T>::pop_back()
	{
		if (empty())
			SG_ASSERT(false && "Can't pop_back an empty vector!");
		else
		{
			T temp = *(mEnd - 1);
			--mEnd;
			(mEnd)->~value_type();
			return temp;
		}
		return T();
	}

	template<typename T>
	template<typename... Args>
	SG_INLINE typename vector<T>::iterator
	vector<T>::emplace_back(Args&&... args)
	{
		if (Size(size()) <= mCapacity)
		{
			*(mEnd) = value_type(SG::forward<Args>(args)...);
			++mEnd;
		}
		else
		{
			double_reserve();
			emplace_back(SG::forward<Args>(args)...);
		}
		return (mEnd - 1);
	}

	template<typename T>
	SG_INLINE T* vector<T>::insert(Size index, const T& value)
	{
		if (index < 0 || index >= Size(size()))
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else if (Size(size()) == mCapacity)
		{
			double_reserve();
		}
		for (Size i = Size(size()) - 1; i > index; i--) // 6 9 (2) 1
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
		if (index < 0 || index >= Size(size()))
		{
			SG_LOG_WARN("index over the boundary!");
			return nullptr;
		}
		else
		{
			for (Size i = index; i < Size(size()) - 1; i++)
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
		Size c= mCapacity >= 256 ? mCapacity + 256 : mCapacity * 2;
		pointer const ptr = reinterpret_cast<pointer>(Calloc(c, sizeof(value_type)));
		SG_ASSERT(ptr && "Failed to allocate memory!");
		mEnd = impl::copy(mBegin, mEnd, ptr);
		if (mBegin)
			Free(mBegin);
		mBegin = ptr;
		mCapacity = c;
	}

	template<typename T>
	SG_INLINE bool vector<T>::operator==(const this_type& rhs) const
	{
		if (Size(size()) != rhs.Size(size()))
			return false;
		else
		{
			for (Size i  override; i < Size(size()); i++)
			{
				if (mBegin[i] != rhs.mBegin[i])
					return false;
			}
		}
		return true;
	}

	template<typename T>
	SG_INLINE typename vector<T>::this_type
	vector<T>::operator=(const this_type& vec)
	{
		if (this != &vec)
		{
			auto c = capacity();
			if (vec.size() > c)
			{
				mBegin = reinterpret_cast<pointer>(ReallocAlign(mBegin, vec.capacity() * sizeof(value_type), 8));
				memcpy(mBegin, vec.mBegin, vec.size() * sizeof(value_type));
				mEnd = mBegin + vec.size();
				mCapacity = vec.mCapacity;
			}
			else
			{
				memcpy(mBegin, vec.mBegin, vec.size() * sizeof(value_type));
				mEnd = mBegin + vec.size();
				mCapacity = vec.mCapacity;
			}
		}
		return *this;
	}

	template<typename T>
	SG_INLINE typename vector<T>::this_type
	vector<T>::operator=(this_type&& vec) noexcept
	{
		if (this != &vec)
		{
			if (mBegin)
				Free(mBegin);
			mBegin = nullptr;
			mEnd = nullptr;
			mCapacity = 0;
			swap(vec);
		}
		return *this;
	}

	template<typename T>
	SG_INLINE typename vector<T>::reference
	vector<T>::operator[](Size index)
	{
#ifdef _DEBUG
		SG_ASSERT(index < Size(size()) && index >= 0 && "Index over the array border!");
#endif
		return *(mBegin + index);
	}

	template<typename T>
	SG_INLINE typename vector<T>::const_reference
	vector<T>::operator[](Size index) const
	{
#ifdef _DEBUG
		SG_ASSERT(index < Size(size()) && index >= 0 && "Index over the array border!");
#endif
		return *(mBegin + index);
	}

	// global swap function
	template <typename T>
	SG_INLINE void swap(vector<T>& a, vector<T>& b) noexcept(noexcept(a.swap(b)))
	{
		a.swap(b);
	}

}

#endif // VECTOR_H