#ifndef STRING_H
#define STRING_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Core/Platform.h"
#include "Common/Base/BasicTypes.h"
#include "Common/Base/IIterable.h"

#include "utility.h"
#include "string_view.h"

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
		template<clsas CharT, size_t = sizeof(T)>
		struct SSOPadding
		{
			char padding[sizeof(T) - sizeof(char)];
		};
		template<clsas CharT>
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

		SG_COMPILE_ASSERT(sizeof(SSOLayout)  == sizeof(HeapLayout), "heap and sso layout structures must be the same size");
		SG_COMPILE_ASSERT(sizeof(HeapLayout) == sizeof(RawDataLayout), "heap and raw layout structures must be the same size");

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
			Layout& operator=(const Layout& other) { Copy(*this, other); return *this; }
			Layout& operator=(Layout&& other)      { Move(*this, other); return *this; }

			//! We are using heap when the bit is set.
			//! Easier to conceptualize checking IsHeap instead of IsSSO
			SG_INLINE bool IsHeap() const noexcept { return !!(sso.mRemainingSizeField.mRemainingSize & ssoMask) };
			SG_INLINE bool IsSSO()  const noexcept { return !IsHeap() };

			// get sso buffer on stack
			SG_INLINE value_type*       SSOBufferPtr() noexcept { return sso.mData; }
			SG_INLINE const value_type* SSOBufferPtr() const noexcept { return sso.mData; }

			//! Max of SSO_CAPACITY is 23 bytes (which is 0001 0111b), which has two LSB bits set (little endian),
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
			SG_INLINE void Move(Layout& dst, Layout& src) noexcept { std::swap(dst.raw, src.raw); }
			SG_INLINE void Swap(Layout& a, Layout& b) noexcept     { std::swap(a.raw, b.raw); }

			//! Reset all the layout memory
			SG_INLINE void ResetToSSO() noexcept { memset(&raw, 0, sizeof(RawDataLayout)); }
		};

		// actual data
		Layout mDataLayout;
	public:
		basic_string() = default;
		explicit basic_string(const value_type* str);
		basic_string(const value_type* str, size_type n);
		basic_string(const value_type* pBeg, const value_type* pEnd);
		~basic_string() = default;
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
		this_type& operator=(this_type&& x);
		this_type& operator=(this_type c);

		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;

		this_type& operator+=(const this_type& x);
		this_type& operator+=(const value_type* p);
		this_type& operator+=(value_type c);

		operator basic_string_view<T>() const noexcept;

		virtual iterator begin() noexcept override               { return mDataLayout.BeginPtr(); }
		virtual const_iterator begin()  const noexcept override  { return mDataLayout.BeginPtr(); }
		virtual const_iterator cbegin() const noexcept override  { return mDataLayout.BeginPtr(); }

		virtual iterator end()   noexcept override            { return mDataLayout.EndPtr(); }
		virtual const_iterator end()  const noexcept override { return mDataLayout.EndPtr(); }
		virtual const_iterator cend() const noexcept override { return mDataLayout.EndPtr(); }

		virtual reverse_iterator rbegin()  noexcept override             { return reverse_iterator(mDataLayout.EndPtr()); }
		virtual const_reverse_iterator rbegin()  const noexcept override { return reverse_iterator(mDataLayout.EndPtr()); }
		virtual const_reverse_iterator crbegin() const noexcept override { return reverse_iterator(mDataLayout.EndPtr()); }

		virtual reverse_iterator rend()  noexcept override             { return reverse_iterator(mDataLayout.BeginPtr()); }
		virtual const_reverse_iterator rend()  const noexcept override { return reverse_iterator(mDataLayout.BeginPtr()); }
		virtual const_reverse_iterator crend() const noexcept override { return reverse_iterator(mDataLayout.BeginPtr()); }
	protected:
		//! Allocate memory depend on SSO or heap
		void DoAllocate(size_type n);
		value_type* CopyCharUninitiazed(value_type* pBeg, value_type* pEnd, value_type* pDst);
	};

	template<class T>
	typename basic_string<T>::this_type
	SG::basic_string<T>::substr(size_type pos, size_type n) const
	{
		SG_ASSERT(pos < mDataLayout.GetSize() && pos >= 0);
		return basic_string(mDataLayout.BeginPtr() + pos, min(n, mDataLayout.GetSize() - pos));
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
			// manually swap, avoid to use std::swap,
			// because it will reversely call T::swap()
			const this_type temp(*this);
			*this = x;
			x = temp;
		}
	}

	template<class T>
	SG_INLINE typename basic_string<T>::value_type*
	SG::basic_string<T>::CopyCharUninitiazed(value_type* pBeg, value_type* pEnd, value_type* pDst)
	{
		// copy data
		const s = Size(pEnd - pBeg);
		memmove(pDst, pBeg, s);
		return pDst + s;
	}

	template<class T>
	SG_INLINE void SG::basic_string<T>::DoAllocate(size_type n)
	{
		// memory allocation
		if (n > SSOLayout::SSO_CAPACITY) // this is not a SSO, allocate on heap
		{
			auto ptr = reinterpret_cast<pointer>(Malloc(n + 1));
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
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* pBeg, const value_type* pEnd)
	{
		const size_type strSize = size_type(pEnd - pBeg);
		DoAllocate(strSize);
		CopyCharUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* str)
	{
		const size_type strSize = len_of_char(str);
		DoAllocate(strSize);
		CopyCharUninitiazed(str, str + strSize, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const value_type* str, size_type n)
	{
		DoAllocate(n);
		CopyCharUninitiazed(str, n, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
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
		CopyCharUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(const this_type& x, size_type position, size_type n /*= npos*/)
	{
		auto pBeg = x.mDataLayout.BeginPtr() + position;
		auto pEnd = x.mDataLayout.BeginPtr() + position + min(n, x.mDataLayout.GetSize() - position);
		const size_type strSize = size_type(pEnd - pBeg);
		DoAllocate(strSize);
		CopyCharUninitiazed(pBeg, pEnd, mDataLayout.BeginPtr());
		mDataLayout.SetSize(s);
		(*mDataLayout.EndPtr()) = 0;
	}

	template<class T>
	SG_INLINE SG::basic_string<T>::basic_string(this_type&& x) noexcept
	{
		mDataLayout = SG::move(x.mDataLayout);
		x.mDataLayout.ResetToSSO();
		x.mDataLayout.SetSSOSize(0);
	}

}

#endif // STRING_H