#ifndef ITERATOR_H
#define ITERATOR_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "../type_traits.h"

namespace SG
{

	/// tags of iterator category
	struct input_iterator_tag  {};
	struct output_iterator_tag {};
	//! on one direction, it is iterable
	struct forward_iterator_tag : public input_iterator_tag {};
	//! both forward and backward, it is iterable
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	//! can be random access
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};
	//! physically contiguous
	struct contiguous_iterator_tag : public random_access_iterator_tag {};

	//! encapsulation of iterator
	template<class Category, class T, class Distance = Diff, class Pointer = T*, class RefType = T&>
	struct iterator
	{
		typedef Category iterator_category;
		typedef T        value_type;
		typedef Distance difference_type;
		typedef Pointer  pointer;
		typedef RefType  reference;
	};

	template<class Iterator>
	struct iterator_traits
	{
		typedef typename Iterator::iterator_category iterator_category;
		typedef typename Iterator::value_type        value_type;
		typedef typename Iterator::difference_type   difference_type;
		typedef typename Iterator::pointer           pointer;
		typedef typename Iterator::reference         reference;
	};

	template<class T>
	struct iterator_traits<T*> // pointer specialization
	{
		typedef SG::random_access_iterator_tag iterator_category;
		typedef T							   value_type;
		typedef Diff						   difference_type;
		typedef T*							   pointer;
		typedef T&							   reference;
	};

	template<class T>
	struct iterator_traits<const T*> // const pointer specialization
	{
		typedef SG::random_access_iterator_tag iterator_category;
		typedef T							   value_type;
		typedef Diff						   difference_type;
		typedef const T*					   pointer;
		typedef const T&					   reference;
	};

	//! SFINAE test for wrapped_iterator_type
	//! Wrapped iterators (e.g. generic_iterator, reverse_iterator, move_iterator)
	//! Non-wrapped iterators (e.g. iterator, char*, int*) 
	template<typename Iterator>
	class is_iterator_wrapper
	{
		template<typename>
		static SG::no_type test(...);

		template<typename U>
		static SG::yes_type test(typename U::wrapped_iterator_type*, typename SG::enable_if<is_class<U>::value>::type* = 0);

	public:
		static const bool value = (sizeof(test<Iterator>(NULL)) == sizeof(yes_type));
	};

	template <typename Iterator, bool isWrapper>
	struct is_iterator_wrapper_impl
	{
		typedef Iterator iterator_type;
		static iterator_type get_base(Iterator iter) { return iter; }
	};

	// If it is a iterator wrapper, it should have iterator_type
	// and base() funciton
	template <typename Iterator>
	struct is_iterator_wrapper_impl<Iterator, true>
	{
		typedef typename Iterator::iterator_type iterator_type;
		static iterator_type get_base(Iterator iter) { return iter.base(); }
	};

	//! Unwrap(get the raw pointer) an iterator if it is a wrapped_iterator_type
	template <typename Iterator>
	SG_INLINE typename is_iterator_wrapper_impl<Iterator, is_iterator_wrapper<Iterator>::value>::iterator_type 
	unwrap_iterator(Iterator it)
	{
		return is_iterator_wrapper_impl<Iterator, is_iterator_wrapper<Iterator>::value>::get_base(it);
	}

	//! Use a simple wrapper to make pointer act like an iterator
	//! To convert an iterable pointer to a formal iterator
	template<class Iterator, class Container>
	class generic_iterator
	{
	public:
		typedef generic_iterator<Iterator, Container>                 this_type;
		typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
		typedef typename iterator_traits<Iterator>::value_type        value_type;
		typedef typename iterator_traits<Iterator>::difference_type   difference_type;
		typedef typename iterator_traits<Iterator>::pointer           pointer;
		typedef typename iterator_traits<Iterator>::reference         reference;
		typedef Iterator                                              iterator_type;
		typedef iterator_type                                         wrapped_iterator_type; // Use to distinguish if this class is a wrapped iterator
		typedef Container                                             container_type;
	protected:
		iterator_type mIterator;
	public:
		/// necessary operator for a pointer to act like an iterator
		generic_iterator()
			: mIterator(iterator_type()) { }

		explicit generic_iterator(const iterator_type& x)
			: mIterator(x) { }

		this_type& operator=(const iterator_type& x)
		{
			mIterator = x; return *this;
		}

		//! Convert to the other types of iterator
		template <typename Iterator2>
		generic_iterator(const generic_iterator<Iterator2, Container>& x)
			: mIterator(x.base()) { }

		reference operator*() const
		{
			return *mIterator;
		}

		pointer operator->() const
		{
			return mIterator;
		}

		this_type& operator++()
		{
			++mIterator; return *this;
		}

		this_type operator++(int)
		{
			return this_type(mIterator++);
		}

		this_type& operator--()
		{
			--mIterator; return *this;
		}

		this_type operator--(int)
		{
			return this_type(mIterator--);
		}

		reference operator[](const difference_type& n) const
		{
			return mIterator[n];
		}

		this_type& operator+=(const difference_type& n)
		{
			mIterator += n; return *this;
		}

		this_type operator+(const difference_type& n) const
		{
			return this_type(mIterator + n);
		}

		this_type& operator-=(const difference_type& n)
		{
			mIterator -= n; return *this;
		}

		this_type operator-(const difference_type& n) const
		{
			return this_type(mIterator - n);
		}

		const iterator_type& base() const
		{
			return mIterator;
		}
	};

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator==(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() == rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator==(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() == rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator!=(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() != rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator!=(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() != rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator<(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() < rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator<(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() < rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator>(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() > rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator>(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() > rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator<=(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() <= rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator<=(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() <= rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE bool operator>=(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() >= rhs.base();
	}
	template <typename Iterator, typename Container>
	SG_INLINE bool operator>=(const generic_iterator<Iterator, Container>& lhs, const generic_iterator<Iterator, Container>& rhs)
	{
		return lhs.base() >= rhs.base();
	}

	template <typename Iterator1, typename Iterator2, typename Container>
	SG_INLINE typename generic_iterator<Iterator1, Container>::difference_type
	operator-(const generic_iterator<Iterator1, Container>& lhs, const generic_iterator<Iterator2, Container>& rhs)
	{
		return lhs.base() - rhs.base();
	}

	/// (utility) check if it is a generic_iterator
	template<class Iterator>
	struct is_generic_iterator : public SG::false_type { };

	template<class Iterator, class Container>
	struct is_generic_iterator<generic_iterator<Iterator, Container>> : public SG::true_type { };

	template<class Iterator>
	SG_CONSTEXPR bool is_generic_iterator_v = is_generic_iterator::value;

	//-------------------------------------------------------------------------------------------------------
	// 	   move_iterator
	//-------------------------------------------------------------------------------------------------------
	 
	//! Just like the generic_iterator 
	//! move_iterator will try to use its dereference operator(operator*())
	//! to implicitly convert the value returned by the underlying iterator's dereference
	//! operator(operator*()) to an right-value(&&)
	//! It will try using moving than copying
	template<typename Iterator>
	class move_iterator
	{
	public:
		typedef Iterator                                iterator_type;
		typedef iterator_traits<Iterator>               traits_type;
		typedef typename traits_type::iterator_category iterator_category;
		typedef typename traits_type::value_type        value_type;
		typedef typename traits_type::difference_type   difference_type;
		typedef iterator_type                           wrapped_iterator_type; // Use to distinguish if this class is a wrapped iterator
		typedef Iterator                                pointer;
		typedef value_type&&                            r_reference;
	protected:
		iterator_type mIterator;
	public:

		move_iterator()
			: mIterator()
		{}

		explicit move_iterator(iterator_type mi)
			: mIterator(mi) { }

		template<typename U>
		move_iterator(const move_iterator<U>& mi)
			: mIterator(mi.base())
		{}

		iterator_type base() const { return mIterator; }

		r_reference operator*() const { return SG::move(*mIterator); }
		pointer     operator->() const { return mIterator; }

		move_iterator& operator++()
		{
			++mIterator;
			return *this;
		}
		move_iterator operator++(int)
		{
			move_iterator tempMoveIterator = *this;
			++mIterator;
			return tempMoveIterator;
		}

		move_iterator& operator--()
		{
			--mIterator;
			return *this;
		}
		move_iterator operator--(int)
		{
			move_iterator tempMoveIterator = *this;
			--mIterator;
			return tempMoveIterator;
		}

		move_iterator operator+(difference_type n) const
		{
			return move_iterator(mIterator + n);
		}
		move_iterator& operator+=(difference_type n)
		{
			mIterator += n;
			return *this;
		}

		move_iterator operator-(difference_type n) const
		{
			return move_iterator(mIterator - n);
		}
		move_iterator& operator-=(difference_type n)
		{
			mIterator -= n;
			return *this;
		}

		r_reference operator[](difference_type n) const
		{
			return SG::move(mIterator[n]);
		}
	};

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator==(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return a.base() == b.base();
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator!=(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return !(a == b);
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator<(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return a.base() < b.base();
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator<=(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return !(b < a);
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator>(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return b < a;
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE bool
	operator>=(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b)
	{
		return !(a < b);
	}

	template<typename Iterator1, typename Iterator2>
	SG_INLINE auto
	operator-(const move_iterator<Iterator1>& a, const move_iterator<Iterator2>& b) -> decltype(a.base() - b.base())
	{
		return a.base() - b.base();
	}

	template<typename Iterator>
	SG_INLINE move_iterator<Iterator>
	operator+(typename move_iterator<Iterator>::difference_type n, const move_iterator<Iterator>& a)
	{
		return a + n;
	}

	template<typename Iterator>
	SG_INLINE move_iterator<Iterator> make_move_iterator(Iterator i)
	{
		return move_iterator<Iterator>(i);
	}

	/// (utility) check if it is a move_iterator
	template<class T>
	struct is_move_iterator : public false_type {};

	template<class Iterator>
	struct is_move_iterator<move_iterator<Iterator>> : public true_type {};

	template<class Iterator>
	SG_CONSTEXPR bool is_move_iterator_v = is_move_iterator<Iterator>::value;

//-------------------------------------------------------------------------------------------------------
// 	   move_iterator
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
// 	   reverse_iterator
//-------------------------------------------------------------------------------------------------------

	//! Reverse iterator is just the opposite of the iterator.
	//! It should have the same structure that the iterator has.
	//! The corresponding relation between iterator and reverse_iterator is 
	//! &*(reverse_iterator(i)) == &*(i - 1).
	template<typename Iterator>
	class reverse_iterator : public iterator<typename iterator_traits<Iterator>::iterator_category,
		typename iterator_traits<Iterator>::value_type,
		typename iterator_traits<Iterator>::difference_type,
		typename iterator_traits<Iterator>::pointer,
		typename iterator_traits<Iterator>::reference>
	{
	public:
		typedef Iterator                                            iterator_type;
		typedef iterator_type                                       wrapped_iterator_type; // Use to distinguish if this class is a wrapped iterator
		typedef typename iterator_traits<Iterator>::pointer         pointer;
		typedef typename iterator_traits<Iterator>::reference       reference;
		typedef typename iterator_traits<Iterator>::difference_type difference_type;
	protected:
		iterator_type mIterator;
	public:
		SG_CONSTEXPR reverse_iterator()
			: mIterator() 
		{}

		SG_CONSTEXPR explicit reverse_iterator(iterator_type i)
			: mIterator(i) 
		{}

		SG_CONSTEXPR reverse_iterator(const reverse_iterator& ri)
			: mIterator(ri.mIterator) 
		{}

		template <typename U>
		SG_CONSTEXPR reverse_iterator(const reverse_iterator<U>& ri)
			: mIterator(ri.base()) 
		{}

		template <typename U>
		SG_CONSTEXPR reverse_iterator<Iterator>& operator=(const reverse_iterator<U>& ri)
		{
			mIterator = ri.base(); return *this;
		}

		SG_CONSTEXPR iterator_type base() const
		{
			return mIterator;
		}

		SG_CONSTEXPR reference operator*() const
		{
			iterator_type i(mIterator);
			return *--i;
		}

		SG_CONSTEXPR pointer operator->() const
		{
			return &(operator*());
		}

		SG_CONSTEXPR reverse_iterator& operator++()
		{
			--mIterator; return *this;
		}
		SG_CONSTEXPR reverse_iterator operator++(int)
		{
			reverse_iterator ri(*this);
			--mIterator;
			return ri;
		}

		SG_CONSTEXPR reverse_iterator& operator--()
		{
			++mIterator; return *this;
		}
		SG_CONSTEXPR reverse_iterator operator--(int)
		{
			reverse_iterator ri(*this);
			++mIterator;
			return ri;
		}

		SG_CONSTEXPR reverse_iterator operator+(difference_type n) const
		{
			return reverse_iterator(mIterator - n);
		}
		SG_CONSTEXPR reverse_iterator& operator+=(difference_type n)
		{
			mIterator -= n; return *this;
		}

		SG_CONSTEXPR reverse_iterator operator-(difference_type n) const
		{
			return reverse_iterator(mIterator + n);
		}
		SG_CONSTEXPR reverse_iterator& operator-=(difference_type n)
		{
			mIterator += n; return *this;
		}

		SG_CONSTEXPR reference operator[](difference_type n) const
		{
			return *(*this + n);
		}
	};

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator==(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() == b.base();
	}

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator<(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() > b.base();
	}

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator!=(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() != b.base();
	}


	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator>(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() < b.base();
	}

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator<=(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() >= b.base();
	}

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE bool
	operator>=(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return a.base() <= b.base();
	}

	template <typename Iterator1, typename Iterator2>
	SG_CONSTEXPR SG_INLINE typename reverse_iterator<Iterator1>::difference_type
	operator-(const reverse_iterator<Iterator1>& a, const reverse_iterator<Iterator2>& b)
	{
		return b.base() - a.base();
	}

	template <typename Iterator>
	SG_CONSTEXPR SG_INLINE reverse_iterator<Iterator>
	operator+(typename reverse_iterator<Iterator>::difference_type n, const reverse_iterator<Iterator>& a)
	{
		return reverse_iterator<Iterator>(a.base() - n);
	}

	/// (utility) check if it is a reverse_iterator
	template<class T>
	struct is_reverse_iterator : public false_type {};

	template<class Iterator>
	struct is_reverse_iterator<reverse_iterator<Iterator>> : public true_type {};

	template<class Iterator>
	SG_CONSTEXPR bool is_reverse_iterator_v = is_reverse_iterator::value;

//-------------------------------------------------------------------------------------------------------
// 	   reverse_iterator
//-------------------------------------------------------------------------------------------------------

} // namespace SG

#endif // COPY_AND_MOVE_H
