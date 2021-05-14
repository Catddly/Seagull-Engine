#ifndef IITERABLE_H
#define IITERABLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SG
{

	//! @Interface
	//! Abstraction for iterable object
	template <typename T>
	struct IIterable
	{
		typedef Size     ptrdiff_t;
		typedef T*		 iterator;
		typedef const T* const_iterator;
		// TODO: add reverse_iterator

		virtual iterator begin() noexcept = 0;
		virtual const_iterator begin()  const noexcept = 0;
		virtual const_iterator cbegin() const noexcept = 0;

		virtual iterator end()   noexcept = 0;
		virtual const_iterator end()  const noexcept = 0;
		virtual const_iterator cend() const noexcept = 0;

		virtual void rbegin()  noexcept = 0;
		virtual void rbegin()  const noexcept = 0;
		virtual void crbegin() const noexcept = 0;

		virtual void rend()  noexcept = 0;
		virtual void rend()  const noexcept = 0;
		virtual void crend() const noexcept = 0;
	};

}

#endif // IITERABLE_H