#pragma once

#include "Base/BasicTypes.h"

#include "Stl/string.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct MemoryInfoBlock
	{
		void*        pAddress = nullptr;
		Size         size = 0;
		string       fileName;
		unsigned int lineNo;
		string       functionName;

		MemoryInfoBlock() = default;
		MemoryInfoBlock(void* ptr, Size s, const string& file, unsigned int l, const string& func)
			:pAddress(ptr), size(s), fileName(file), lineNo(l), functionName(func)
		{}
	};

	class MemoryLeakDetecter
	{
	public:
		~MemoryLeakDetecter()
		{
			GetInstance()->DumpLeak();
		}

		template <typename T, typename... Args>
		T* RecordNew(const string& file, unsigned int l, const string& func, Args&&... args)
		{
			auto* p = RecordMalloc(sizeof(T), file, l, func);
			new (p) T(eastl::forward<Args>(args)...);
			return reinterpret_cast<T*>(p);
		}

		template <typename T>
		void RecordDelete(T* ptr)
		{
			ptr->~T();
			RecordFree(ptr);
		}

		SG_CORE_API void* RecordMalloc(Size size, const string& file, unsigned line, const string& func);
		SG_CORE_API void* RecordRealloc(void* ptr, Size size, const string& file, unsigned line, const string& func);
		SG_CORE_API void  RecordFree(void* ptr);

		SG_CORE_API void DumpLeak() const;

		SG_CORE_API static MemoryLeakDetecter* GetInstance();
	private:
		MemoryLeakDetecter() = default;
	private:
		eastl::unordered_map<void*, MemoryInfoBlock> mMemoryBlocks;
	};

}