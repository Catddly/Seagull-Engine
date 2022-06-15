#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "Memory/MemoryTracker.h"
#include "Memory/MemoryLeakDetecter.h"

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
	namespace Memory
	{
		namespace Impl
		{
#if SG_ENABLE_MEMORY_TRACKING
			SG_CORE_API void* Malloc(Size size, const char* file, UInt32 line, const char* function) noexcept;
#endif
			SG_CORE_API void* MallocInternal(Size size) noexcept;
			SG_CORE_API void* MallocAlignInternal(Size size, Size alignment) noexcept;
			SG_CORE_API void* CallocInternal(Size count, Size size) noexcept;
			SG_CORE_API void* ReallocInternal(void* ptr, Size newSize) noexcept;
			SG_CORE_API void* ReallocAlignInternal(void* ptr, Size newSize, Size alignment) noexcept;

			SG_CORE_API void  FreeInternal(void* ptr) noexcept;
			SG_CORE_API void  FreeAlignInternal(void* ptr, Size alignment) noexcept;

			template <typename T, typename... Args>
			T* NewInternal(Args&&... args) noexcept
			{
				auto* p = MallocInternal(sizeof(T));
				new (p) T(eastl::forward<Args>(args)...);
				return reinterpret_cast<T*>(p);
			}

			template <typename T>
			void DeleteInternal(T* ptr) noexcept
			{
				ptr->~T();
				FreeInternal(ptr);
			}
		}
	};

#if SG_ENABLE_MEMORY_LEAK_DETECTION
#	define Malloc(SIZE)       SG::MemoryLeakDetecter::GetInstance()->RecordMalloc(SIZE, __FILE__, __LINE__, __FUNCTION__)
#	define Realloc(PTR, SIZE) SG::MemoryLeakDetecter::GetInstance()->RecordRealloc(PTR, SIZE, __FILE__, __LINE__, __FUNCTION__)
#	define Free(PTR)          SG::MemoryLeakDetecter::GetInstance()->RecordFree(PTR)

#	define New(TYPE, ...)     SG::MemoryLeakDetecter::GetInstance()->RecordNew<TYPE>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#	define Delete(PTR)        SG::MemoryLeakDetecter::GetInstance()->RecordDelete(PTR);
#else						  
#	define Malloc(SIZE)       SG::Memory::Impl::MallocInternal(SIZE)
#	define Realloc(PTR, SIZE) SG::Memory::Impl::ReallocInternal(PTR, SIZE)
#	define Free(PTR)          SG::Memory::Impl::FreeInternal(PTR)

#	define New(TYPE, ...)     SG::Memory::Impl::NewInternal<TYPE>(__VA_ARGS__)
#	define Delete(PTR)        SG::Memory::Impl::DeleteInternal(PTR)
#endif

#if SG_ENABLE_MEMORY_TRACKING
#	define MallocTrack(TYPE) reinterpret_cast<TYPE*>(Memory::Malloc(sizeof(TYPE), __FILE__, __LINE__, __FUNCSIG__))
#else
#	define MallocTrack(TYPE) reinterpret_cast<TYPE*>(Malloc(sizeof(TYPE)))
#endif

}