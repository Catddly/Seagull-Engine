#ifndef SPAN_H
#define SPAN_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

#include "internal/iterator.h"
#include "type_traits.h"

namespace SG
{
	template<typename T>
	class span
	{
	public:
		typedef span<T>  this_type;
		typedef T        value_type;
		typedef T*       pointer;
		typedef T&       reference;
		typedef const T* const_pointer;
		typedef const T& const_reference;
		typedef T*       iterator;
		typedef const T* const_iterator;
		typedef SG::reverse_iterator<iterator> reverse_iterator;
		typedef SG::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef Size     size_type;
	protected:
		const_pointer   mBegin;
		size_type       mCount;
	public:
		SG_CONSTEXPR span() noexcept : mBegin(nullptr), mCount(0) {}
		SG_CONSTEXPR span(const span&) noexcept = default;
		span& operator=(const span&) = default;
		SG_CONSTEXPR span(const_pointer ptr, size_type n) : mBegin(ptr), mCount(n) {}
		SG_CONSTEXPR span(const_pointer ptr) : mBegin(ptr) { mCount = size_of_array<const_pointer>::value; }
	
		SG_CONSTEXPR void swap(span& rhs)
		{
			std::swap(mBegin, rhs.mBegin);
			std::swap(mCount, rhs.mCount);
		}

		SG_CONSTEXPR size_type     size()  const noexcept { return mCount; }
		SG_CONSTEXPR const_pointer data()  const noexcept { return mBegin; }
		SG_CONSTEXPR bool          empty() const noexcept { return mCount == 0; }

		SG_CONSTEXPR const_iterator begin()  const noexcept { return mBegin; }
		SG_CONSTEXPR const_iterator cbegin() const noexcept { return mBegin; }

		SG_CONSTEXPR const_iterator end()  const noexcept { return mBegin + mCount; }
		SG_CONSTEXPR const_iterator cend() const noexcept { return mBegin + mCount; }

		SG_CONSTEXPR const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(mBegin + mCount); }
		SG_CONSTEXPR const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(mBegin + mCount); }

		SG_CONSTEXPR const_reverse_iterator rend()  const noexcept { return const_reverse_iterator(mBegin); }
		SG_CONSTEXPR const_reverse_iterator crend() const noexcept { return const_reverse_iterator(mBegin); }

		SG_CONSTEXPR const_reference operator[](size_type pos) const { return mBegin[pos]; }
	};

}

#endif // SPAN_H
