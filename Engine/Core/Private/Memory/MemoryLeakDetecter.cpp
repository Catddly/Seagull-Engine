#include "StdAfx.h"
#include "Memory/MemoryLeakDetecter.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	void* MemoryLeakDetecter::RecordMalloc(Size size, const string& file, unsigned line, const string& func)
	{
		void* ptr = Memory::Impl::MallocInternal(size);
		SG_ASSERT(ptr);
		auto node = mMemoryBlocks.find(ptr);
		if (node != mMemoryBlocks.end())
		{
			printf("Memory malloc collide detected at 0x%p (%s, %u, %s)\n", ptr, file.c_str(), line, func.c_str());
			SG_ASSERT(false);
		}
		mMemoryBlocks[ptr] = MemoryInfoBlock(ptr, size, file, line, func);
		return ptr;
	}

	void* MemoryLeakDetecter::RecordRealloc(void* ptr, Size size, const string& file, unsigned line, const string& func)
	{
		if (!ptr)
			return nullptr;

		auto node = mMemoryBlocks.find(ptr);
		if (node == mMemoryBlocks.end())
		{
			printf("Memory frees a invalid pointer detected at 0x%p\n", ptr);
			SG_ASSERT(false);
		}
		mMemoryBlocks.erase(node);

		void* newPtr = Memory::Impl::ReallocInternal(ptr, size);

		SG_ASSERT(newPtr);
		node = mMemoryBlocks.find(newPtr);
		if (node != mMemoryBlocks.end())
		{
			printf("Memory malloc collide detected at 0x%p (%s, %u, %s)\n", newPtr, file.c_str(), line, func.c_str());
			SG_ASSERT(false);
		}
		mMemoryBlocks[ptr] = MemoryInfoBlock(newPtr, size, file, line, func);
		return ptr;
	}

	void MemoryLeakDetecter::RecordFree(void* ptr)
	{
		if (!ptr)
			return;

		auto node = mMemoryBlocks.find(ptr);
		if (node == mMemoryBlocks.end())
		{
			printf("Memory frees a invalid pointer detected at 0x%p\n", ptr);
			SG_ASSERT(false);
		}
		mMemoryBlocks.erase(node);
		Memory::Impl::FreeInternal(ptr);
	}

	void MemoryLeakDetecter::DumpLeak() const
	{
		if (mMemoryBlocks.empty())
			printf("No memory leak detected!\n");
		else
		{
			for (auto node : mMemoryBlocks)
			{
				auto& block = node.second;
				printf("Memory leak detected at 0x%p (%zu) (%s, %u, %s)\n", block.pAddress, block.size, block.fileName.c_str(), block.lineNo, block.functionName.c_str());
			}
		}
	}

	MemoryLeakDetecter* MemoryLeakDetecter::GetInstance()
	{
		static MemoryLeakDetecter sInstance;
		return &sInstance;
	}

}