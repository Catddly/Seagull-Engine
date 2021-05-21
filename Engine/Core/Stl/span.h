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
		typedef span<T>        this_type;
		typedef T              element_type;
		typedef remove_cv_t<T> value_type;
		typedef T*             pointer;
		typedef T&             reference;
		typedef const T*       const_pointer;
		typedef const T&       const_reference;
		typedef T*             iterator;
		typedef const T*       const_iterator;
		typedef SG::reverse_iterator<iterator> reverse_iterator;
		typedef SG::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef Size     size_type;
	protected:
		pointer    mBegin;
		size_type  mCount;
	public:
		SG_CONSTEXPR span() noexcept : mBegin(nullptr), mCount(0) {}
		SG_CONSTEXPR span(const span&) noexcept = default;
		span& operator=(const span&) noexcept = default;
		SG_CONSTEXPR span(pointer ptr,  size_type n) : mBegin(ptr), mCount(n) {}
		SG_CONSTEXPR span(pointer pBeg, pointer pEnd) : mBegin(pBeg), mCount(size_type(pEnd - pBeg)) {}
		~span() noexcept = default;

		// for compile-time deduction
		template<size_t N> SG_CONSTEXPR span(element_type(&arr)[N]) noexcept : span(arr, static_cast<size_type>(N)) {}

		SG_CONSTEXPR pointer        data()  const noexcept { return mBegin; }
		SG_CONSTEXPR size_type      size()  const noexcept { return mCount; }
		SG_CONSTEXPR bool           empty() const noexcept { return mCount == 0; }

		// subview
		SG_CONSTEXPR span<element_type> first(size_type count) const;
		SG_CONSTEXPR span<element_type> last(size_type count) const;
		SG_CONSTEXPR span<element_type> subspan(size_type offset, size_type count) const;

		SG_CONSTEXPR const_iterator begin()  const noexcept { return mBegin; }
		SG_CONSTEXPR const_iterator cbegin() const noexcept { return mBegin; }

		SG_CONSTEXPR const_iterator end()  const noexcept { return mBegin + mCount; }
		SG_CONSTEXPR const_iterator cend() const noexcept { return mBegin + mCount; }

		SG_CONSTEXPR const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(mBegin + mCount); }
		SG_CONSTEXPR const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(mBegin + mCount); }

		SG_CONSTEXPR const_reverse_iterator rend()  const noexcept { return const_reverse_iterator(mBegin); }
		SG_CONSTEXPR const_reverse_iterator crend() const noexcept { return const_reverse_iterator(mBegin); }

		SG_CONSTEXPR reference operator[](size_type pos) const;
		SG_CONSTEXPR reference operator()(size_type pos) const;
	private:
		SG_CONSTEXPR bool bound_check(size_type n) const;
	};

	template<typename T>
	SG_INLINE SG_CONSTEXPR span<T> span<T>::first(size_type count) const
	{
		return span{mBegin, count > mCount ? mCount : count};
	}

	template<typename T>
	SG_INLINE SG_CONSTEXPR span<T> span<T>::last(size_type count) const
	{
		count = count > mCount ? mCount : count;
		return span{mBegin + mCount - count, count};
	}

	template<typename T>
	SG_INLINE SG_CONSTEXPR span<T> span<T>::subspan(size_type offset, size_type count) const
	{
		SG_ASSERT(bound_check(offset) && "Exceed the boundary!");
		count = count > mCount - offset ? mCount - offset : count;
		return span{mBegin + offset, count};
	}

	template<typename T>
	SG_INLINE SG_CONSTEXPR typename span<T>::reference 
	span<T>::operator[](size_type pos) const
	{
		SG_ASSERT(bound_check(pos) && "Exceed the boundary!");
		return mBegin[pos];
	}

	template<typename T>
	SG_INLINE SG_CONSTEXPR typename span<T>::reference
	span<T>::operator()(size_type pos) const
	{
		SG_ASSERT(bound_check(pos) && "Exceed the boundary!");
		return mBegin[pos];
	}

	template<typename T>
	SG_INLINE SG_CONSTEXPR bool span<T>::bound_check(size_type n) const
	{
		return ((n >= 0) && (n < mCount));
	}

}

#endif // SPAN_H
