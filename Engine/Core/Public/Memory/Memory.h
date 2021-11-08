#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "Memory/MemoryTracker.h"

#define SG_ENABLE_MEMORY_TRACKING 0

//struct nothrow_t
//{
//	explicit nothrow_t() = default;
//};
//enum class align_val_t : size_t {};

// overload global new and delete
//void* operator new(size_t n);
//void* operator new(size_t n, const nothrow_t& tag); // C++ 11
//void* operator new(size_t n, align_val_t align);    // C++ 17
//void* operator new[](size_t n);
//void* operator new[](size_t n, const nothrow_t& tag); // C++ 11
//void* operator new[](size_t n, align_val_t align);    // C++ 17
//
//void  operator delete(void* p) noexcept;
//void  operator delete(void* p, size_t n) noexcept;
//void  operator delete(void* p, const nothrow_t& tag) noexcept; // only be called when new_nothrow is been called
//void  operator delete(void* ptr, align_val_t align) noexcept;
//void  operator delete[](void* p) noexcept;
//void  operator delete[](void* p, size_t n) noexcept;
//void  operator delete[](void* p, const nothrow_t& tag) noexcept;
//void  operator delete[](void* ptr, align_val_t align) noexcept;

namespace SG
{
	//! Functor to do all the memory allocation job.
	struct Memory
	{
		Memory() = delete;
		~Memory() = delete;

#if defined(SG_ENABLE_MEMORY_TRACKING)
		SG_CORE_API static void* Malloc(Size size, const char* file, UInt32 line, const char* function) noexcept;
#endif

		SG_CORE_API static void* Malloc(Size size) noexcept;
		SG_CORE_API static void* MallocAlign(Size size, Size alignment) noexcept;
		SG_CORE_API static void* Calloc(Size count, Size size) noexcept;
		SG_CORE_API static void* CallocAlign(Size count, Size size, Size alignment) noexcept;
		SG_CORE_API static void* Realloc(void* ptr, Size newSize) noexcept;
		SG_CORE_API static void* ReallocAlign(void* ptr, Size newSize, Size alignment) noexcept;

		SG_CORE_API static void  Free(void* ptr) noexcept;
		SG_CORE_API static void  FreeAlign(void* ptr, Size alignment) noexcept;

		template <class T, class... Args>
		static T* New(Args&&... args);

		template <class T, class... Args>
		static T* NewAlign(Args&&... args);

		template <class T>
		static void Delete(T* ptr);

		template <class T>
		static void DeleteAlign(T* ptr);
	};

	template <class T, class...Args>
	T* Memory::New(Args&&... args)
	{
		auto p = Malloc(sizeof(T));
		new (p) T(eastl::forward<Args>(args)...);
		return reinterpret_cast<T*>(p);
	}

	template <class T, class...Args>
	T* Memory::NewAlign(Args&&... args)
	{
		auto p = MallocAlign(sizeof(T), (Size)alignof(T));
		new (p) T(eastl::forward<Args>(args)...);
		return reinterpret_cast<T*>(p);
	}

	template <class T>
	void Memory::Delete(T* ptr)
	{
		ptr->~T();
		Free(ptr);
	}

	template <class T>
	void Memory::DeleteAlign(T* ptr)
	{
		ptr->~T();
		FreeAlign(ptr, (Size)alignof(T));
	}


#if defined(SG_ENABLE_MEMORY_TRACKING)
#	define MallocTrack(TYPE) reinterpret_cast<TYPE*>(Memory::Malloc(sizeof(TYPE), __FILE__, __LINE__, __FUNCSIG__))
#endif

}