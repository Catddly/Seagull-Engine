#ifndef IITERABLE_H
#define IITERABLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/STL/internal/iterator.h"

namespace SG
{

	//! @Interface
	//! Abstraction for iterable object
	template <typename T>
	struct IIterable
	{
		typedef Diff     difference_type;
		typedef T*		 iterator;
		typedef const T* const_iterator;
		typedef SG::reverse_iterator<iterator>       reverse_iterator;
		typedef SG::reverse_iterator<const_iterator> const_reverse_iterator;

		virtual iterator begin() noexcept = 0;
		virtual const_iterator begin()  const noexcept = 0;
		virtual const_iterator cbegin() const noexcept = 0;

		virtual iterator end()   noexcept = 0;
		virtual const_iterator end()  const noexcept = 0;
		virtual const_iterator cend() const noexcept = 0;

		virtual reverse_iterator rbegin()  noexcept = 0;
		virtual const_reverse_iterator rbegin()  const noexcept = 0;
		virtual const_reverse_iterator crbegin() const noexcept = 0;

		virtual reverse_iterator rend()  noexcept = 0;
		virtual const_reverse_iterator rend()  const noexcept = 0;
		virtual const_reverse_iterator crend() const noexcept = 0;
	};

}

#endif // IITERABLE_H