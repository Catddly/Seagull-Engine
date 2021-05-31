#ifndef STRING_H
#define STRING_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Core/Platform.h"
#include "Common/Base/BasicTypes.h"
#include "Common/Base/IIterable.h"
#include "Common/Memory/IMemory.h"

#include "utility.h"
#include "algorithm.h"
#include "string_view.h"
#include "internal/vs_printf.h"

#include <utility>
#include <cstdarg>

namespace SG
{

	template<class T>
	class basic_string : public IIterable<T>
	{
	public:
		typedef basic_string<T>      this_type;
		typedef basic_string_view<T> view_type;
		typedef T                    value_type;
		typedef T*                   pointer;
		typedef const T*             const_pointer;
		typedef T&                   reference;
		typedef const T&             const_reference;
		typedef Size				 size_type;

		using IIterable::difference_type;
		using IIterable::iterator;
		using IIterable::const_iterator;
		using IIterable::reverse_iterator;
		using IIterable::const_reverse_iterator;

		//! non-valid position in string
		static const size_type npos = size_type(-1);
	protected:
		//! The view of memory for storing string data
		struct HeapLayout
		{
			value_type* mBegin;
			size_type   mSize;
			size_type   mCapacity;
		};

		//! This is for buffer cache hit rate.
		//! If T's size of byte if more than sizeof(char), add the rest of the byte
		//! as padding to ensure the memory is consistent
		template<class CharT, size_t = sizeof(T)>
		struct SSOPadding
		{
			char padding[sizeof(T) - sizeof(char)];
		};
		template<class CharT>
		struct SSOPadding<CharT, 1> // when the char type is 1 bite(char), no padding
		{
			// template specialization to remove the padding structure to avoid warnings on zero length arrays
			// also, this allows us to take advantage of the empty-base-class optimization.
		};

		//! The layout of SSO(Short String Optimization)
		//! The view of memory when the string data is able to store locally
		//! No Heap Allocation
		struct SSOLayout
		{
			//! The capacity of SSO
			//! (e.g. for string, HeapLayout is 24 bytes, so SSO_CAPACITY is 23)
			//! the max size of SSO_CAPACITY is 23 bytes
			SG_CONSTEXPR static size_type SSO_CAPACITY = (sizeof(HeapLayout) - sizeof(char)) / sizeof(value_type);
			
			// mSize must correspond to the last byte of HeapLayout.mCapacity, so we don't want the compiler to insert
			// padding after mSize if sizeof(value_type) != 1.
			// Also ensures both layouts are the same size.
			struct SSOSize : SSOPadding<value_type>
			{
				char mRemainingSize;
			};

			value_type mData[SSO_CAPACITY]; // local buffer for string data. (e.g. for string, it is char[23] which is 23 bytes)
			SSOSize    mRemainingSizeField; // padding                       (e.g. for string, it is 1 byte)
		};

		//! Raw buffer of data for easy copying 
		struct RawDataLayout
		{
			char mBuffer[sizeof(HeapLayout)];
		};

		// Masks used to determine if we are in SSO or Heap
#ifdef SG_LITTLE_ENDIAN
	    // Little Endian use MSB
		SG_CONSTEXPR static size_type heapMask = ~(size_type(~size_type(0)) >> 1); // 1000 0000 0000 0000b (the MSB for 8 byte)
		SG_CONSTEXPR static size_type ssoMask  = 0x80;                             // 1000 0000b           (the MSB for 4 byte)
#else
		// Big Endian use LSB, unless we want to reorder struct layouts on endianess, Bit is set when we are in Heap.
		SG_CONSTEXPR static size_type heapMask = 0x1;
		SG_CONSTEXPR static size_type ssoMask  = 0x1;
#endif

		SG_COMPILE_ASSERT(sizeof(SSOLayout)  == sizeof(HeapLayout) && "heap and sso layout structures must be the same size");
		SG_COMPILE_ASSERT(sizeof(HeapLayout) == sizeof(RawDataLayout) && "heap and raw layout structures must be the same size");

		//! The Implementation of SSO(Short String Optimization)
		//! SSO reuses the existing string class to hold the string data which is short enough
		//! to fit therefore avoiding the frequent heap allocation.
		//! The number of characters stored in the string SSO buffer is variable and depends on the string character width. 
		//! This implementation favors a consistent string size than increasing the size of 
		//! the string local data to accommodate a consistent number of characters despite character width.
		struct Layout
		{
			union
			{
				HeapLayout	  heap;
				SSOLayout	  sso;
				RawDataLayout raw;
			};

			Layout() { ResetToSSO(); SetSSOSize(0); } // start as SSO by default
			Layout(const Layout& other) { Copy(*this, other); }
			Layout(Layout&& other)      { Move(*this, other); }
			Layout& operator=(const Layout& other) noexcept { Copy(*this, other); return *this; }
			Layout& operator=(Layout&& other)      noexcept { Move(*this, other); return *this; }

			//! We are using heap when the bit is set.
			//! Easier to conceptualize checking IsHeap instead of IsSSO
			SG_INLINE bool IsHeap() const noexcept { return !!(sso.mRemainingSizeField.mRemainingSize & ssoMask); };
			SG_INLINE bool IsSSO()  const noexcept { return !IsHeap(); };

			// get sso buffer on stack
			SG_INLINE value_type*       SSOBufferPtr() noexcept { return sso.mData; }
			SG_INLINE const value_type* SSOBufferPtr() const noexcept { return sso.mData; }

			//! max of SSO_CAPACITY is 23 bytes (which is 0001 0111b), which has two LSB bits set (little endian),
			//! but on bit endian we still use LSB to denote heap, 23 bytes is (1101 0100b), so shift 2.
			SG_INLINE size_type GetSSOSize() const noexcept 
			{
#ifdef SG_LITTLE_ENDIAN
				return (SSOLayout::SSO_CAPACITY - sso.mRemainingSizeField.mRemainingSize);        // little endian
#else
				return (SSOLayout::SSO_CAPACITY - (sso.mRemainingSizeField.mRemainingSize >> 2)); // big endian
#endif
			}
			//! New sso size should be the remaining size of the stack (0001 0110b) (0101 1000b)
			SG_INLINE void      SetSSOSize(size_type size) noexcept
			{
#ifdef SG_LITTLE_ENDIAN
				sso.mRemainingSizeField.mRemainingSize = (char)(SSOLayout::SSO_CAPACITY - size);
#else
				sso.mRemainingSizeField.mRemainingSize = (char)((SSOLayout::SSO_CAPACITY - size) << 2);
#endif
			}

			SG_INLINE size_type GetHeapSize() const noexcept { return heap.mSize; }
			SG_INLINE void      SetHeapSize(size_type size) noexcept { heap.mSize = size; }

			SG_INLINE size_type GetSize()     const noexcept { return IsHeap() ? GetHeapSize() : GetSSOSize(); }
			SG_INLINE void      SetSize(size_type size) noexcept { IsHeap() ? SetHeapSize(size) : SetSSOSize(size); }

			SG_INLINE size_type GetHeapCapacity() const noexcept
			{
#ifdef SG_LITTLE_ENDIAN
				return (heap.mCapacity & ~heapMask);
#else
				return (heap.mCapacity >> 1);
#endif
			}
			SG_INLINE void      SetHeapCapacity(size_type cap) noexcept
			{
#ifdef SG_LITTLE_ENDIAN
				heap.mCapacity = (cap | heapMask);
#else
				heap.mCapacity = (cap << 1) | heapMask;
#endif
			}
			SG_INLINE size_type GetRemainingCapacity() const noexcept { return size_type(CapacityPtr() - EndPtr()); }

			// points to end of the buffer at the terminating '\0', *ptr == '\0' <- not true for SSO
			SG_INLINE value_type*       CapacityPtr() noexcept { return IsHeap() ? HeapCapacityPtr() : SSOCapcityPtr(); }
			SG_INLINE const value_type* CapacityPtr() const noexcept { return IsHeap() ? HeapCapacityPtr() : SSOCapcityPtr(); }

			SG_INLINE void SetHeapBeginPtr(value_type* pBegin) noexcept { heap.mBegin = pBegin; }

			SG_INLINE value_type*       HeapBeginPtr() noexcept { return heap.mBegin; };
			SG_INLINE const value_type* HeapBeginPtr() const noexcept { return heap.mBegin; };

			SG_INLINE value_type*       SSOBeginPtr() noexcept { return sso.mData; }
			SG_INLINE const value_type* SSOBeginPtr() const noexcept { return sso.mData; }

			SG_INLINE value_type*       BeginPtr() noexcept { return IsHeap() ? HeapBeginPtr() : SSOBeginPtr(); }
			SG_INLINE const value_type* BeginPtr() const noexcept { return IsHeap() ? HeapBeginPtr() : SSOBeginPtr(); }

			SG_INLINE value_type*       HeapEndPtr() noexcept { return heap.mBegin + heap.mSize; }
			SG_INLINE const value_type* HeapEndPtr() const noexcept { return heap.mBegin + heap.mSize; }

			SG_INLINE value_type*       SSOEndPtr() noexcept { return sso.mData + GetSSOSize(); }
			SG_INLINE const value_type* SSOEndPtr() const noexcept { return sso.mData + GetSSOSize(); }

			//! Points to end of character stream, *ptr == '\0'
			SG_INLINE value_type*       EndPtr() noexcept { return IsHeap() ? HeapEndPtr() : SSOEndPtr(); }
			//! Points to end of character stream, *ptr == '\0'
			SG_INLINE const value_type* EndPtr() const noexcept { return IsHeap() ? HeapEndPtr() : SSOEndPtr(); }

			SG_INLINE value_type*       HeapCapacityPtr() noexcept { return heap.mBegin + GetHeapCapacity(); }
			SG_INLINE const value_type* HeapCapacityPtr() const noexcept { return heap.mBegin + GetHeapCapacity(); }

			SG_INLINE value_type*       SSOCapcityPtr() noexcept { return sso.mData + SSOLayout::SSO_CAPACITY; }
			SG_INLINE const value_type* SSOCapcityPtr() const noexcept { return sso.mData + SSOLayout::SSO_CAPACITY; }

			// use the RawDataLayout structure, we can easily do modification to the memory
			SG_INLINE void Copy(Layout& dst, const Layout& src) noexcept { dst.raw = src.raw; }
			SG_INLINE void Move(Layout& dst, Layout& src) noexcept       { std::swap(dst.raw, src.raw); }
			SG_INLINE void Swap(Layout& a, Layout& b) noexcept           { std::swap(a.raw, b.raw); }

			//! Reset all the layout memory
			SG_INLINE void ResetToSSO() noexcept { memset(&raw, 0, sizeof(RawDataLayout)); }
		};

		// actual data
		Layout mDataLayout;
	public:
		//! Tag for printf like initialized to string
		struct CtorSprintf {};

		//! Tag for string object to build (memory allocation),
		//! no size initialization
		struct CtorDoNotInitialize {};

		basic_string() { mDataLayout.ResetToSSO(); mDataLayout.SetSSOSize(0); };
		basic_string(const value_type* str);
		basic_string(const value_type* str, size_type n);
		basic_string(const value_type* pBeg, const value_type* pEnd);
		basic_string(CtorDoNotInitialize, size_type n);
		basic_string(CtorSprintf, const value_type* pFormat, ...);
		~basic_string();
		// copy ctor
		basic_string(const this_type& x);
		basic_string(const this_type& x, size_type position, size_type n = npos);
		// move ctor
		basic_string(this_type&& x) noexcept;
		// for string_view
		explicit basic_string(const view_type& sv);
		basic_string(const view_type& sv, size_type position, size_type n);

		void swap(this_type& x);

		bool      empty() const noexcept { return mDataLayout.GetSize() == 0; }
		size_type size() const noexcept { return mDataLayout.GetSize(); }
		size_type length() const noexcept { return mDataLayout.GetSize(); }
		size_type capacity() const noexcept { return mDataLayout.IsHeap() ? mDataLayout.GetHeapCapacity() : SSOLayout::SSO_CAPACITY; }

		void clear() noexcept;

		this_type substr(size_type pos = 0, size_type n = npos) const;
		const value_type* data() const noexcept { return mDataLayout.BeginPtr(); }
		const value_type* c_str() const noexcept { return mDataLayout.BeginPtr(); }

		this_type& operator=(const value_type* p);
		this_type& operator=(const this_type& x);
		this_type& operator=(view_type v);
		this_type& operator=(this_type&& x) noexcept;
		this_type& operator=(value_type c);

		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;

		this_type& operator+=(const this_type& x);
		this_type& operator+=(const value_type* p);
		this_type& operator+=(value_type c);

		operator basic_string_view<T>() const noexcept;

		// main assign
		this_type& assign(const value_type* pBegin, const value_type* pEnd);
		this_type& assign(const this_type& rhs);
		this_type& assign(this_type&& rhs);
		this_type& assign(const value_type* str);
		this_type& assign(const value_type* str, size_type n);
		this_type& assign(size_type n, value_type c);

		// main insert
		iterator   insert(const_iterator p, const value_type* pBegin, const value_type* pEnd);
		this_type& insert(size_type position, const value_type* p);
		
		// main erase
		iterator erase(const_iterator pBegin, const_iterator pEnd);

		// main append
		this_type& append(const value_type* pBeg, const value_type* pEnd);
		this_type& append(const value_type* pBeg);
		this_type& append(const this_type& x);
		this_type& append(size_type n, value_type c);

		// main compare
		static int compare(const value_type* pBeg1, const value_type* pEnd1, const value_type* pBeg2, const value_type* pEnd2);

		// main find
		size_type find(const value_type* ptr, size_type pos, size_type n) const;
		size_type find(value_type c, size_type pos = 0) const noexcept;
		size_type find(const value_type* ptr, size_type pos = 0) const;
		size_type find(const this_type& x, size_type pos = 0) const noexcept;

		// main find_first_of
		size_type find_first_of(const value_type* ptr, size_type pos, size_type n) const;
		size_type find_first_of(value_type c, size_type pos = 0) const noexcept;
		size_type find_first_of(const value_type* ptr, size_type pos = 0) const;
		size_type find_first_of(const this_type& x, size_type pos = 0) const noexcept;

		void resize(size_type n);
		void reserve(size_type n);
		//! Reset the capacity to n
		void set_capacity(size_type n = npos);

		void push_back(value_type c);

		virtual iterator begin() noexcept override               { return mDataLayout.BeginPtr(); }
		virtual const_iterator begin()  const noexcept override  { return mDataLayout.BeginPtr(); }
		virtual const_iterator cbegin() const noexcept override  { return mDataLayout.BeginPtr(); }

		virtual iterator end()   noexcept override            { return mDataLayout.EndPtr(); }
		virtual const_iterator end()  const noexcept override { return mDataLayout.EndPtr(); }
		virtual const_iterator cend() const noexcept override { return mDataLayout.EndPtr(); }

		virtual reverse_iterator rbegin()  noexcept override             { return reverse_iterator(mDataLayout.EndPtr()); }
		virtual const_reverse_iterator rbegin()  const noexcept override { return const_reverse_iterator(mDataLayout.EndPtr()); }
		virtual const_reverse_iterator crbegin() const noexcept override { return const_reverse_iterator(mDataLayout.EndPtr()); }

		virtual reverse_iterator rend()  noexcept override             { return reverse_iterator(mDataLayout.BeginPtr()); }
		virtual const_reverse_iterator rend()  const noexcept override { return const_reverse_iterator(mDataLayout.BeginPtr()); }
		virtual const_reverse_iterator crend() const noexcept override { return const_reverse_iterator(mDataLayout.BeginPtr()); }
	protected:
		//! Allocate memory depend on SSO or heap.
		void DoAllocate(size_type n);
		//! Deallocate memory depend on heap (If it is stack, do nothing).
		void DoDeallocate();
		//! Safely copy string of chars.
		value_type* CopyCharPtrUninitiazed(const value_type* pBeg, const value_type* pEnd, value_type* pDst);
		//! DoubleReserved if using heap, otherwise use the SSO_CAPACITY.
		size_type DoubleReserved(size_type currCapacity);
		//! Expand args and append it to the formatted string.
		this_type& AppendSprintfVaList(const value_type* format, va_list args);
		//! Find first same value of two strings [pBeg1, pEnd1) and [pBeg2, pEnd2).
		//! Complexity: O(n^2) depend on the length of two strings
		value_type* CharTypeStringFindFirstOf(const value_type* pBeg1, const value_type* pEnd1, const value_type* pBeg2, const value_type* pEnd2);
	};

	template<class T>
	SG_INLINE typename SG::basic_string<T>::value_type* 
	SG::basic_string<T>::CharTypeStringFindFirstOf(const value_type* pBeg1, const value_type* pEnd1, const value_type* pBeg2, const value_type* pEnd2)
	{
		for (; pBeg1 != pEnd1; ++pBeg1)
		{
			for (const value_type* pTemp = pBeg2; pTemp != pEnd2; ++pTemp)
			{
				if (*pBeg1 == *pTemp)
					return pBeg1;
			}
		}
		return pEnd1;
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::clear() noexcept
	{
		mDataLayout.SetSize(0);
		*mDataLayout.BeginPtr() = value_type(0);
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::set_capacity(size_type n /*= npos*/)
	{
		if (n == npos)
		{
			n = mDataLayout.GetSize();
		}
		else if (n < mDataLayout.GetSize())
		{
			mDataLayout.SetSize(n);
			*mDataLayout.EndPtr() = 0;
		}

		if ((n < capacity() && mDataLayout.IsHeap()) || (n > capacity())) // need heap reallocate
		{
			if (n != 0)
			{
				if (n <= SSOLayout::SSO_CAPACITY) // heap to SSO
				{
					pointer oldBegPtr = mDataLayout.BeginPtr();
					const size_type oldCap = mDataLayout.GetHeapCapacity();

					mDataLayout.ResetToSSO(); // clear up the memory
					CopyCharPtrUninitiazed(oldBegPtr, oldBegPtr + n, mDataLayout.BeginPtr());
					mDataLayout.SetSSOSize(n);
					Free(oldBegPtr); // free the heap allocated memory
					return;
				}
				
				// may be SSO to heap or heap to heap
				pointer newBegPtr = reinterpret_cast<pointer>(Malloc((n + 1) * sizeof(value_type)));
				const size_type oldSize = mDataLayout.GetSize();
				pointer newEndPtr = CopyCharPtrUninitiazed(mDataLayout.BeginPtr(), mDataLayout.EndPtr(), newBegPtr);
				*newEndPtr = 0;
				// deallocate memory depend on it is heap or SSO
				DoDeallocate();
				mDataLayout.SetHeapBeginPtr(newBegPtr);
				mDataLayout.SetHeapCapacity(n);
				mDataLayout.SetHeapSize(oldSize);
			}
			else
			{
				DoDeallocate();
				mDataLayout.ResetToSSO();
				mDataLayout.SetSSOSize(0);
			}
		}
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::reserve(size_type n)
	{
		n = smax(n, mDataLayout.GetSize());
		if (n > capacity())
			set_capacity(n);
	}
	
	template<class T>
	SG_INLINE void SG::basic_string<T>::resize(size_type n)
	{
		const size_type currSize = mDataLayout.GetSize();
		if (n < currSize)
			erase(mDataLayout.BeginPtr() + n, mDataLayout.EndPtr());
		else if (n > currSize)
		{
			// for build-in types, value_type() is same as (value_type(0))
			// for compound types, it will call the default ctor
			append(n - currSize, value_type());
		}
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type
	SG::basic_string<T>::DoubleReserved(size_type currCapacity)
	{
		return currCapacity <= SSOLayout::SSO_CAPACITY ? SSOLayout::SSO_CAPACITY : (currCapacity * 2);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::AppendSprintfVaList(const value_type* format, va_list args)
	{
		const size_type initializedSize = mDataLayout.GetSize();
		int appendedSize;

		appendedSize = Vsnprintf(nullptr, 0, format, args); // return the length of the appended string
		
		if (appendedSize > 0)
		{
			resize(initializedSize + appendedSize);
			// actual append the expend formatted string to the data
			appendedSize = Vsnprintf(mDataLayout.BeginPtr() + initializedSize, (size_type)(appendedSize + 1ull), format, args);
		}

		if (appendedSize >= 0)
		{
			mDataLayout.SetSize(initializedSize + appendedSize);
		}

		return *this;
	}

	template <typename T>
	SG_INLINE void basic_string<T>::push_back(value_type c)
	{
		append((size_type)1, c);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find(const value_type* ptr, size_type pos, size_type n) const
	{
		if ((pos + n) <= mDataLayout.GetSize() && (npos - n) >= pos) // if pos is valid
		{
			const value_type* const ptr = SG::search(mDataLayout.BeginPtr() + pos, mDataLayout.EndPtr(), ptr, ptr + n);
			if ((ptr != mDataLayout.EndPtr()) || (n == 0))
				return (size_type)(ptr - mDataLayout.BeginPtr());
		}
		return npos;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find(value_type c, size_type pos) const noexcept
	{
		if (pos < mDataLayout.GetSize()) // if the position is valid
		{
			const const_iterator pResult = SG::find(mDataLayout.BeginPtr() + pos, mDataLayout.EndPtr(), c);

			if (pResult != mDataLayout.EndPtr())
				return (size_type)(pResult - mDataLayout.BeginPtr());
		}
		return npos;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find(const value_type* ptr, size_type pos /*= 0*/) const
	{
		return find(ptr, pos, (size_type)len_of_char_str(ptr));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find(const this_type& x, size_type pos /*= 0*/) const noexcept
	{
		return find(x.mDataLayout.BeginPtr(), position, x.mDataLayout.GetSize());
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find_first_of(value_type c, size_type pos) const noexcept
	{
		return find(c, pos);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find_first_of(const value_type* ptr, size_type position, size_type n) const
	{
		if (pos < mDataLayout.GetSize())
		{
			const value_type* const pBeg = mDataLayout.BeginPtr() + pos;
			const const_iterator pRes = CharTypeStringFindFirstOf(pBeg, mDataLayout.EndPtr(), ptr, ptr + n);

			if (pRes != mDataLayout.EndPtr())
				return (size_type)(pRes - mDataLayout.BeginPtr());
		}
		return npos;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type 
	SG::basic_string<T>::find_first_of(const this_type& x, size_type pos /*= 0*/) const noexcept
	{
		return find_first_of(x.mDataLayout.BeginPtr(), pos, x.mDataLayout.GetSize());
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::size_type
	SG::basic_string<T>::find_first_of(const value_type* ptr, size_type pos /*= 0*/) const
	{
		return find_first_of(ptr, pos, (size_type)len_of_char_str(ptr));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::append(const value_type* pBeg, const value_type* pEnd)
	{
		if (pBeg != pEnd)
		{
			const size_type appendSize = size_type(pEnd - pBeg);
			const size_type currSize = mDataLayout.GetSize();
			const size_type cap = capacity();

			if (appendSize + currSize > cap) // do heap reallocation
			{
				const size_type newCapacity = SG::smax(DoubleReserved(cap), currSize + appendSize);

				pointer newBegPtr = reinterpret_cast<pointer>(Malloc((newCapacity + 1) * sizeof(value_type)));
				auto newEndPtr = CopyCharPtrUninitiazed(mDataLayout.BeginPtr(), mDataLayout.EndPtr(), newBegPtr); // copy the origin part
				newEndPtr = CopyCharPtrUninitiazed(pBeg, pEnd, newEndPtr); // copy the appended part
				*newEndPtr = 0;

				DoDeallocate();
				mDataLayout.SetHeapBeginPtr(newBegPtr);
				mDataLayout.SetHeapCapacity(newCapacity);
				mDataLayout.SetHeapSize(appendSize + currSize);
			}
			else // no need to allocate on heap
			{
				pointer newEndPtr = CopyCharPtrUninitiazed(pBeg, pEnd, mDataLayout.EndPtr());
				*newEndPtr = 0;
				mDataLayout.SetSize(currSize + appendSize);
			}
		}
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::append(const value_type* pBeg)
	{
		return append(pBeg, pBeg + len_of_char_str(pBeg));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::append(size_type n, value_type c)
	{
		const size_type oldSize = mDataLayout.GetSize();
		const size_type cap = capacity();

		if (oldSize + n > cap)
			reserve(smax(DoubleReserved(cap), (oldSize + n)));

		if (n > 0)
		{
			pointer newEndPtr = assign_char_n(mDataLayout.EndPtr(), n, c);
			*newEndPtr = 0;
			mDataLayout.SetSize(oldSize + n);
		}
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::append(const this_type& x)
	{
		return append(x.mDataLayout.BeginPtr(), x.mDataLayout.EndPtr());
	}

	template<class T>
	SG_INLINE int SG::basic_string<T>::compare(const value_type* pBeg1, const value_type* pEnd1, const value_type* pBeg2, const value_type* pEnd2)
	{
		const difference_type n1 = pEnd1 - pBeg1;
		const difference_type n2 = pEnd2 - pBeg2;
		const size_type nMin = (size_type)SG::smin(n1, n2);

		const int res = SG::compare(pBeg1, pBeg2, nMin);

		if (res != 0)
			return res;
		else if (n1 < n2) // if the common part of the string is same, compare the length of the string
			return -1;
		else if (n1 > n2)
			return 1;
		else
			return 0;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::assign(const this_type& rhs)
	{
		return assign(rhs.mDataLayout.BeginPtr(), rhs.mDataLayout.EndPtr());
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::assign(this_type&& rhs)
	{
		assign(rhs.mDataLayout.BeginPtr(), rhs.mDataLayout.EndPtr());
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::assign(const value_type* str)
	{
		return assign(str, str + len_of_char_str(str));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::assign(const value_type* str, size_type n)
	{
		return assign(str, str + n);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::assign(size_type n, value_type c)
	{
		if (n <= mDataLayout.GetSize())
		{
			assign_char_n(mDataLayout.BeginPtr(), n, c);
			erase(mDataLayout.BeginPtr() + n, mDataLayout.EndPtr());
		}
		else
		{
			assign_char_n(mDataLayout.BeginPtr(), mDataLayout.GetSize(), c);
			append(c, );
		}
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::assign(const value_type* pBegin, const value_type* pEnd)
	{
		const size_type n = size_type(pEnd - pBegin);
		if (n <= mDataLayout.GetSize()) // delete the rest part of the origin data
		{
			memmove(mDataLayout.BeginPtr(), pBegin, n * sizeof(value_type));
			erase(mDataLayout.BeginPtr() + n, mDataLayout.EndPtr());
		}
		else // append the rest part if assigned data
		{
			memmove(mDataLayout.BeginPtr(), pBegin, (size_type)mDataLayout.GetSize() * sizeof(value_type));
			append(pBegin + mDataLayout.GetSize(), pEnd);
		}
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::iterator 
	SG::basic_string<T>::insert(const_iterator ptr, const value_type* pBegin, const value_type* pEnd)
	{
		const difference_type beginPos = (ptr - mDataLayout.BeginPtr()); // restore the begin pointer's offset
		SG_ASSERT(ptr >= mDataLayout.BeginPtr() && ptr < mDataLayout.EndPtr() && "Invalid pointer position!");
		const size_type insertSize = size_type(pEnd - pBegin);
		
		if (insertSize != 0)
		{
			bool bIsCapacitySufficient = (mDataLayout.GetRemainingCapacity() >= n);
			bool bIsSourceFromSelf = ((pEnd >= mDataLayout.BeginPtr()) && (pBegin <= mDataLayout.EndPtr()));

			if (bIsSourceFromSelf && mDataLayout.IsSSO()) // on stack data inside itself
			{
				// because the source is from self and it is a SSO string,
				// so stackStringTemp is a SSO string to.
				const this_type stackStringTemp(pBegin, pEnd);
				// may be there will be 1 or 2 allocations happened, 
				// so we recursively call insert to insert the copied data to *this.
				return insert(ptr, stackStringTemp.data(), stackStringTemp.data() + stackStringTemp.size());
			}

			if (bIsCapacitySufficient && !bIsSourceFromSelf) // have enough capacity to copy the data
			{
				const size_type leftSize = size_type(mDataLayout.EndPtr() - ptr);

				// if there are enough characters between begin + pos and end
				if (leftSize >= insertSize) // enough
				{
					const size_type currSize = mDataLayout.GetSize();
					CopyCharPtrUninitiazed((mDataLayout.EndPtr() - insertSize) + 1, mDataLayout.EndPtr() + 1, mDataLayout.EndPtr() + 1); // copy the last part of the string to the end
					mDataLayout.SetSize(currSize + insertSize);
					memmove(const_cast<value_type*>(ptr) + insertSize, ptr, (size_type)((leftSize - insertSize) + 1) * sizeof(value_type));
					memmove(const_cast<value_type*>(ptr), pBegin, (size_type)(insertSize) * sizeof(value_type));
				}
				else // not enough
				{
					pointer oldEndPtr = mDataLayout.EndPtr();
					const value_type* const pMid = pBegin + (leftSize + 1);

					CopyCharPtrUninitiazed(pMid, pEnd, mDataLayout.EndPtr() + 1);
					mDataLayout.SetSize(mDataLayout.GetSize() + (insertSize - leftSize));

					const size_type currSize = mDataLayout.GetSize();
					CopyCharPtrUninitiazed(p, oldEndPtr + 1, mDataLayout.EndPtr());
					mDataLayout.SetSize(currSize + insertSize);

					CopyCharPtrUninitiazed(pBegin, pMid, const_cast<value_type*>(p));
				}
			}
			else // we need to reallocate
			{
				const size_type currSize = mDataLayout.GetSize();
				const size_type cap = capacity();
				size_type nLength;

				if (bIsCapacitySufficient) // If bCapacityIsSufficient is true, then bSourceIsFromSelf must be true.
					nLength = currSize + n;
				else
					nLength = smax(DoubleReserved(cap), (currSize + n));

				pointer newBegPtr = reinterpret_cast<pointer>(Malloc((nLength + 1) * sizeof(value_type)));

				pointer newEndPtr = CopyCharPtrUninitiazed(mDataLayout.BeginPtr(), p, newBegPtr);
				newEndPtr = CopyCharPtrUninitiazed(pBegin, pEnd, newEndPtr);
				newEndPtr = CopyCharPtrUninitiazed(p, mDataLayout.EndPtr(), newEndPtr);
				*newEndPtr = 0;

				DoDeallocate();
				mDataLayout.SetHeapBeginPtr(newBegPtr);
				mDataLayout.SetHeapCapacity(nLength);
				mDataLayout.SetHeapSize(currSize + n);
			}
		}

		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::insert(size_type position, const value_type* p)
	{
		return insert(mDataLayout.BeginPtr() + position, p, p + len_of_char_str(p));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::iterator
	SG::basic_string<T>::erase(const_iterator pBegin, const_iterator pEnd)
	{
		SG_ASSERT(pBegin >= mDataLayout.BeginPtr() && pBegin <= mDataLayout.EndPtr() &&
			pEnd >= mDataLayout.BeginPtr() && pEnd <= mDataLayout.EndPtr() && pBegin <= pEnd && "invalid iterator position!");
		if (pBegin != pEnd)
		{
			const size_type erasedSize = size_type(pEnd - pBegin);
			const size_type currSize = mDataLayout.GetSize();
			memmove(const_cast<value_type*>(pBegin), pEnd, ((size_type)(mDataLayout.EndPtr() - pEnd) + 1) * sizeof(value_type)); // move the back part to the front
			mDataLayout.SetSize(currSize - erasedSize);
		}
		return const_cast<value_type*>(pBegin);
	}

	template<class T>
	typename basic_string<T>::this_type
	SG::basic_string<T>::substr(size_type pos, size_type n) const
	{
		SG_ASSERT(pos < mDataLayout.GetSize() && pos >= 0);
		return basic_string(mDataLayout.BeginPtr() + pos, smin(n, mDataLayout.GetSize() - pos));
	}

	template<class T>
	void SG::basic_string<T>::swap(this_type& x)
	{
		if (mDataLayout.IsSSO() && x.mDataLayout.IsSSO()) // on stack
		{
			std::swap(mDataLayout, x.mDataLayout);
		}
		else
		{
			// manually swap, avoid to use SG::swap,
			// because it will call T::swap()
			const this_type temp(*this);
			*this = x;
			x = temp;
		}
	}

	template<class T>
	SG_INLINE typename basic_string<T>::value_type*
	SG::basic_string<T>::CopyCharPtrUninitiazed(const value_type* pBeg, const value_type* pEnd, value_type* pDst)
	{
		// copy data
		const size_type s = size_type(pEnd - pBeg);
		memmove(pDst, pBeg, s * sizeof(T));
		return pDst + s;
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::DoAllocate(size_type n)
	{
		// memory allocation
		if (n > SSOLayout::SSO_CAPACITY) // this is not a SSO, allocate on heap
		{
			auto ptr = reinterpret_cast<pointer>(Malloc((n + 1) * sizeof(value_type)));
			mDataLayout.SetHeapBeginPtr(ptr);
			mDataLayout.SetHeapCapacity(n);
			mDataLayout.SetHeapSize(0);
		}
		else // we can use sso
		{
			mDataLayout.ResetToSSO();
			mDataLayout.SetSSOSize(0);
		}
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::DoDeallocate()
	{
		if (mDataLayout.IsHeap())
		{
			Free(mDataLayout.BeginPtr());
		}
	}

	template<class T>
	SG::basic_string<T>::basic_string(CtorDoNotInitialize cdni, size_type n)
	{
		SG_NO_USE(cdni);
		DoAllocate(n);
		// no set mDataLayout.SetSize(n) here, just have the memory ready
		*mDataLayout.EndPtr() = 0;
	}

	template<class T>
	SG::basic_string<T>::basic_string(CtorSprintf spf, const value_type* pFormat, ...)
	{
		SG_NO_USE(spf);
		const size_type size = (size_type)len_of_char_str(pFormat);
		DoAllocate(size);

		va_list args;
		va_start(args, pFormat);
		AppendSprintfVaList(pFormat, args);
		va_end(args);
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* pBeg, const value_type* pEnd)
	{
		const size_type strSize = size_type(pEnd - pBeg);
		DoAllocate(strSize);
		CopyCharPtrUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* str)
	{
		const size_type strSize = len_of_char_str(str);
		DoAllocate(strSize);
		CopyCharPtrUninitiazed(str, str + strSize, mDataLayout.BeginPtr());
		mDataLayout.SetSize(strSize);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* str, size_type n)
	{
		DoAllocate(n);
		CopyCharPtrUninitiazed(str, str + n, mDataLayout.BeginPtr());
		mDataLayout.SetSize(n);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const view_type& sv)
		: basic_string(sv.data(), sv.length())
	{}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const view_type& sv, size_type position, size_type n)
		: basic_string(sv.substr(position, n))
	{}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const this_type& x)
	{
		auto pBeg = x.mDataLayout.BeginPtr();
		auto pEnd = x.mDataLayout.EndPtr();
		const size_type strSize = size_type(pEnd - pBeg);
		DoAllocate(strSize);
		CopyCharPtrUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(strSize);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG::basic_string<T>::basic_string(const this_type& x, size_type position, size_type n /*= npos*/)
	{
		auto pBeg = x.mDataLayout.BeginPtr() + position;
		auto pEnd = x.mDataLayout.BeginPtr() + position + min(n, x.mDataLayout.GetSize() - position);
		const size_type strSize = size_type(pEnd - pBeg);
		DoAllocate(strSize);
		CopyCharPtrUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG::basic_string<T>::basic_string(this_type&& x) noexcept
	{
		mDataLayout = SG::move(x.mDataLayout);
		x.mDataLayout.ResetToSSO();
		x.mDataLayout.SetSSOSize(0);
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::~basic_string()
	{
		DoDeallocate();
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator=(const value_type* str)
	{
		return assign(str, str + len_of_char_str(str));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator=(const this_type& x)
	{
		assign(x.mDataLayout.BeginPtr(), x.mDataLayout.EndPtr());
		return *this;
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator=(view_type v)
	{
		return assign(v.data(), static_cast<this_type::size_type>(v.length()));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator=(this_type&& x) noexcept
	{
		return assign(SG::move(x));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator=(value_type c)
	{
		return assign(c, 1);
	}

	template<class T>
	basic_string<T>::operator basic_string_view<T>() const noexcept
	{
		return basic_string_view<T>(data(), size());
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::const_reference 
	SG::basic_string<T>::operator[](size_type n) const
	{
		SG_ASSERT(n < mDataLayout.GetSize() && "index exceed the boundary!");
		return *(mDataLayout.BeginPtr() + n);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::reference
	SG::basic_string<T>::operator[](size_type n)
	{
		SG_ASSERT(n < mDataLayout.GetSize() && "index exceed the boundary!");
		return *(mDataLayout.BeginPtr() + n);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::operator+=(value_type c)
	{
		return append(1, c);
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type&
	SG::basic_string<T>::operator+=(const value_type* str)
	{
		return append(str, str + len_of_char_str(str));
	}

	template<class T>
	SG_INLINE typename SG::basic_string<T>::this_type& 
	SG::basic_string<T>::operator+=(const this_type& x)
	{
		return append(x.mDataLayout.BeginPtr(), x.mDataLayout.EndPtr());
	}

	/// global operators
	template <typename T>
	SG_INLINE bool operator==(const typename basic_string<T>::reverse_iterator& r1,
		const typename basic_string<T>::reverse_iterator& r2)
	{
		return r1.mIterator == r2.mIterator;
	}

	template <typename T>
	SG_INLINE bool operator!=(const typename basic_string<T>::reverse_iterator& r1,
		const typename basic_string<T>::reverse_iterator& r2)
	{
		return r1.mIterator != r2.mIterator;
	}

	template <typename T>
	basic_string<T> operator+(const basic_string<T>& a, const basic_string<T>& b)
	{
		typedef typename basic_string<T>::CtorDoNotInitialize CtorDoNotInitialize;
		CtorDoNotInitialize cDNI; // GCC 2.x forces us to declare a named temporary like this.
		basic_string<T> result(cDNI, a.size() + b.size()); // no size assignment, just allocate the memory
		result.append(a);
		result.append(b);
		return result;
	}
	template <typename T>
	basic_string<T> operator+(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		typedef typename basic_string<T>::CtorDoNotInitialize CtorDoNotInitialize;
		CtorDoNotInitialize cDNI; // GCC 2.x forces us to declare a named temporary like this.
		const typename basic_string<T>::size_type n = (typename basic_string<T>::size_type)len_of_char_str(p);
		basic_string<T> result(cDNI, n + b.size());
		result.append(p, p + n);
		result.append(b);
		return result;
	}
	template <typename T>
	basic_string<T> operator+(typename basic_string<T>::value_type c, const basic_string<T>& b)
	{
		typedef typename basic_string<T>::CtorDoNotInitialize CtorDoNotInitialize;
		CtorDoNotInitialize cDNI; // GCC 2.x forces us to declare a named temporary like this.
		basic_string<T> result(cDNI, 1 + b.size());
		result.push_back(c);
		result.append(b);
		return result;
	}
	template <typename T>
	basic_string<T> operator+(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		typedef typename basic_string<T>::CtorDoNotInitialize CtorDoNotInitialize;
		CtorDoNotInitialize cDNI; // GCC 2.x forces us to declare a named temporary like this.
		const typename basic_string<T>::size_type n = (typename basic_string<T>::size_type)len_of_char_str(p);
		basic_string<T> result(cDNI, a.size() + n);
		result.append(a);
		result.append(p, p + n);
		return result;
	}
	template <typename T>
	basic_string<T> operator+(const basic_string<T>& a, typename basic_string<T>::value_type c)
	{
		typedef typename basic_string<T>::CtorDoNotInitialize CtorDoNotInitialize;
		CtorDoNotInitialize cDNI; // GCC 2.x forces us to declare a named temporary like this.
		basic_string<T> result(cDNI, a.size() + 1);
		result.append(a);
		result.push_back(c);
		return result;
	}
	template <typename T>
	basic_string<T> operator+(basic_string<T>&& a, basic_string<T>&& b)
	{
		a.append(b); // b is right value, && + & is &.
		return SG::move(a);
	}
	template <typename T>
	basic_string<T> operator+(basic_string<T>&& a, const basic_string<T>& b)
	{
		a.append(b);
		return SG::move(a);
	}
	template <typename T>
	basic_string<T> operator+(const typename basic_string<T>::value_type* p, basic_string<T>&& b)
	{
		b.insert(0, p);
		return SG::move(b);
	}
	template <typename T>
	basic_string<T> operator+(basic_string<T>&& a, const typename basic_string<T>::value_type* p)
	{
		a.append(p);
		return SG::move(a);
	}
	template <typename T>
	basic_string<T> operator+(basic_string<T>&& a, typename basic_string<T>::value_type c)
	{
		a.push_back(c);
		return SG::move(a);
	}

	template <typename T>
	SG_INLINE bool operator==(const basic_string<T>& a, const basic_string<T>& b)
	{
		return ((a.size() == b.size()) && (memcmp(a.data(), b.data(), (Size)a.size() * sizeof(typename basic_string<T>::value_type)) == 0));
	}
	template <typename T>
	SG_INLINE bool operator==(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		typedef typename basic_string<T>::size_type size_type;
		const size_type n = (size_type)len_of_char_str(p);
		return ((n == b.size()) && (memcmp(p, b.data(), (size_t)n * sizeof(*p)) == 0));
	}
	template <typename T>
	SG_INLINE bool operator==(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		typedef typename basic_string<T>::size_type size_type;
		const size_type n = (size_type)len_of_char_str(p);
		return ((a.size() == n) && (memcmp(a.data(), p, (size_t)n * sizeof(*p)) == 0));
	}
	template <typename T>
	SG_INLINE bool operator!=(const basic_string<T>& a, const basic_string<T>& b)
	{
		return !(a == b);
	}
	template <typename T>
	SG_INLINE bool operator!=(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		return !(p == b);
	}
	template <typename T>
	SG_INLINE bool operator!=(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		return !(a == p);
	}

	template <typename T>
	SG_INLINE bool operator<(const basic_string<T>& a, const basic_string<T>& b)
	{
		return basic_string<T>::compare(a.begin(), a.end(), b.begin(), b.end()) < 0;
	}
	template <typename T>
	SG_INLINE bool operator<(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		typedef typename basic_string<T>::size_type size_type;
		const size_type n = (size_type)len_of_char_str(p);
		return basic_string<T>::compare(p, p + n, b.begin(), b.end()) < 0;
	}
	template <typename T>
	SG_INLINE bool operator<(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		typedef typename basic_string<T>::size_type size_type;
		const size_type n = (size_type)len_of_char_str(p);
		return basic_string<T>::compare(a.begin(), a.end(), p, p + n) < 0;
	}

	template <typename T>
	SG_INLINE bool operator>(const basic_string<T>& a, const basic_string<T>& b)
	{
		return b < a;
	}
	template <typename T>
	SG_INLINE bool operator>(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		return b < p;
	}
	template <typename T>
	SG_INLINE bool operator>(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		return p < a;
	}

	template <typename T>
	SG_INLINE bool operator<=(const basic_string<T>& a, const basic_string<T>& b)
	{
		return !(b < a);
	}
	template <typename T>
	SG_INLINE bool operator<=(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		return !(b < p);
	}
	template <typename T>
	SG_INLINE bool operator<=(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		return !(p < a);
	}

	template <typename T>
	SG_INLINE bool operator>=(const basic_string<T>& a, const basic_string<T>& b)
	{
		return !(a < b);
	}
	template <typename T>
	SG_INLINE bool operator>=(const typename basic_string<T>::value_type* p, const basic_string<T>& b)
	{
		return !(p < b);
	}
	template <typename T>
	SG_INLINE bool operator>=(const basic_string<T>& a, const typename basic_string<T>::value_type* p)
	{
		return !(a < p);
	}

	/// global swap function
	template <typename T>
	SG_INLINE void swap(basic_string<T>& a, basic_string<T>& b) noexcept(noexcept(a.swap(b)))
	{
		a.swap(b);
	}

	/// definitions
	typedef basic_string<Char>   string;
	typedef basic_string<WChar>  wstring;

	typedef basic_string<Char8>  u8string;
	typedef basic_string<Char16> u16string;
	typedef basic_string<Char32> u32string;

	/// to_string() fucntions, users can define thiers
	SG_INLINE string to_string(Int32 num) { return string{string::CtorSprintf(), "%d", num}; }
	SG_INLINE string to_string(long num) { return string{string::CtorSprintf(), "%ld", num}; }
	SG_INLINE string to_string(Int64 num) { return string{string::CtorSprintf(), "%lld", num}; }
	SG_INLINE string to_string(UInt32 num) { return string{string::CtorSprintf(), "%u", num}; }
	SG_INLINE string to_string(unsigned long num) { return string{string::CtorSprintf(), "%lu", num}; }
	SG_INLINE string to_string(UInt64 num) { return string{string::CtorSprintf(), "%llu", num}; }
	SG_INLINE string to_string(Float32 num) { return string{string::CtorSprintf(), "%f", num}; }
	SG_INLINE string to_string(Float64 num) { return string{string::CtorSprintf(), "%f", num}; }
	SG_INLINE string to_string(LFloat num) { return string{string::CtorSprintf(), "%Lf", num}; }

	SG_INLINE wstring to_wstring(Int32 num) { return wstring{ wstring::CtorSprintf(), L"%d", num }; }
	SG_INLINE wstring to_wstring(long num) { return wstring{ wstring::CtorSprintf(), L"%ld", num }; }
	SG_INLINE wstring to_wstring(Int64 num) { return wstring{ wstring::CtorSprintf(), L"%lld", num }; }
	SG_INLINE wstring to_wstring(UInt32 num) { return wstring{ wstring::CtorSprintf(), L"%u", num }; }
	SG_INLINE wstring to_wstring(unsigned long num) { return wstring{ wstring::CtorSprintf(), L"%lu", num }; }
	SG_INLINE wstring to_wstring(UInt64 num) { return wstring{ wstring::CtorSprintf(), L"%llu", num }; }
	SG_INLINE wstring to_wstring(Float32 num) { return wstring{ wstring::CtorSprintf(), L"%f", num }; }
	SG_INLINE wstring to_wstring(Float64 num) { return wstring{ wstring::CtorSprintf(), L"%f", num }; }
	SG_INLINE wstring to_wstring(LFloat num) { return wstring{ wstring::CtorSprintf(), L"%Lf", num }; }

	SG_DISABLE_MSVC_WARNING(4455) // disable warning: 'operator ""s': literal suffix identifiers that do not start with an underscore are reserved.
	inline namespace literals
	{
		inline namespace string_view_literals
		{
			SG_INLINE string    operator ""s(const Char* str, Size len) noexcept { return { str, string::size_type(len) }; }
			SG_INLINE wstring   operator ""s(const WChar* str, Size len) noexcept { return { str, wstring::size_type(len) }; }
			SG_INLINE u16string operator ""s(const Char16* str, Size len) noexcept { return { str, u16string::size_type(len) }; }
			SG_INLINE u32string operator ""s(const Char32* str, Size len) noexcept { return { str, u32string::size_type(len) }; }
		}
	}
	SG_RESTORE_MSVC_WARNING()
}

#endif // STRING_H