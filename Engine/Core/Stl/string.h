#ifndef STRING_H
#define STRING_H

#ifdef _MSC_VER
#	pragma once
#endif

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "Common/Base/IIterable.h"

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
		static const size_type pos = size_type(-1);
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
		template<CharT, size_t = sizeof(T)>
		struct SSOPadding
		{
			char padding[sizeof(T) - sizeof(char)];
		};
		template<CharT>
		struct SSOPadding<CharT, 1>
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
			SG_CONSTEXPR static size_type SSO_CAPACITY = (sizeof(HeapLayout) - sizeof(char)) / sizeof(value_type);
			
			// mSize must correspond to the last byte of HeapLayout.mCapacity, so we don't want the compiler to insert
			// padding after mSize if sizeof(value_type) != 1.
			// Also ensures both layouts are the same size.
			struct SSOSize : SSOPadding<value_type>
			{
				char mRemainingSize;
			};

			value_type mData[SSO_CAPACITY]; // local buffer for string data.
			SSOSize    mRemainingSizeField; // padding
		};

		//! Raw buffer of data for easy copying 
		struct RawDataLayout
		{
			char mBuffer[sizeof(HeapLayout)];
		};

		// Masks used to determine if we are in SSO or Heap
#ifdef SG_SYSTEM_BIG_ENDIAN
		// Big Endian use LSB, unless we want to reorder struct layouts on endianness, Bit is set when we are in Heap
		static constexpr size_type kHeapMask = 0x1;
		static constexpr size_type kSSOMask = 0x1;
#else
		// Little Endian use MSB (use top bit )
		SG_CONSTEXPR static size_type kHeapMask = ~(size_type(~size_type(0)) >> 1);
		SG_CONSTEXPR static size_type kSSOMask = 0x80; // 1000 0000b
#endif

		SG_COMPILE_ASSERT(sizeof(SSOLayout) == sizeof(HeapLayout), "heap and sso layout structures must be the same size");
		SG_COMPILE_ASSERT(sizeof(HeapLayout) == sizeof(RawDataLayout), "heap and raw layout structures must be the same size");

		//! The Implementation of SSO(Short String Optimization)
		//! SSO reuses the existing string class to hold the string data which is short enough
		//! to fit therefore aviding the frequent heap allocation.
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

			//! We are using heap when the bit is set.
			//! Easier to conceptualize checking IsHeap instead of IsSSO
			SG_INLINE bool IsHeap() const noexcept { return !!(sso.mRemainingSizeField & ) };
		};
	};

}

#endif // STRING_H