#include "StdAfx.h"
#include "Memory/MemoryTracker.h"

#include "System/Logger.h"

namespace SG
{

	MemoryScopeTracker::MemoryScopeTracker(const char* scopeName)
#ifdef SG_DEBUG
		:mName(scopeName)
#endif
	{
		MemoryRecorder::OnEnterScope(scopeName);
	}

	MemoryScopeTracker::~MemoryScopeTracker()
	{
		MemoryRecorder::OnExitScope();
	}

	eastl::pair<const char*, MemoryTrackBlock*> MemoryRecorder::mTrackStack[MAX_SCOPE] = {};
	Int32             MemoryRecorder::mCurrStackPos = -1;
	MemoryTrackBlock* MemoryRecorder::mpForgottenMemory = (MemoryTrackBlock*)mi_malloc(sizeof(MemoryTrackBlock));

	void MemoryRecorder::TrackMemory(void* memory, Size size, const char* file, UInt32 line, const char* function)
	{
		if (mCurrStackPos == -1)
			return;

		auto* newBlock = (MemoryTrackBlock*)mi_malloc(sizeof(MemoryTrackBlock));
		newBlock->file = file;
		newBlock->line = line;
		newBlock->function = function;
		newBlock->memory = memory;
		newBlock->size = size;

		//SG_LOG_DEBUG("(Scope: %s) Memory Allocated: %zu bytes", mTrackStack[mCurrStackPos].first, size);

		Insert(newBlock, mTrackStack[mCurrStackPos].second);
	}

	void MemoryRecorder::ReleaseMemory(void* memory)
	{
		if (mCurrStackPos == -1)
			return;

		Remove(memory);
	}

	void MemoryRecorder::Insert(MemoryTrackBlock* pBlock, MemoryTrackBlock* pEnd)
	{
		pBlock->prev = pEnd->prev;
		pEnd->prev = pBlock;
	}

	void MemoryRecorder::Remove(void* pMemory)
	{
		auto* pEnd = mTrackStack[mCurrStackPos].second;
		MemoryTrackBlock* pPrev = pEnd;
		MemoryTrackBlock* pCurr = pEnd->prev;
		while (pCurr)
		{
			if (pCurr->memory == pMemory)
			{
				//SG_LOG_DEBUG("(Scope: %s) Memory Freed: %zu bytes", mTrackStack[mCurrStackPos].first, pCurr->size);
				pPrev->prev = pCurr->prev; // erase pCurr
				mi_free(pCurr);
				return;
			}

			pPrev = pCurr;
			pCurr = pCurr->prev;
		}

		pCurr = mpForgottenMemory->prev;
		while (pCurr)
		{
			if (pCurr->memory == pMemory)
			{
				//SG_LOG_DEBUG("(Scope: %s) Memory Freed: %zu bytes", mTrackStack[mCurrStackPos].first, pCurr->size);
				mpForgottenMemory->prev = pCurr->prev; // erase pCurr
				mi_free(pCurr);
				return;
			}

			pCurr = pCurr->prev;
		}
		SG_LOG_ERROR("Unpredicted memory!");
		SG_ASSERT(false);
	}

	void MemoryRecorder::OnEnterScope(const char* scopeName)
	{
		auto* pEnd = (MemoryTrackBlock*)mi_malloc(sizeof(MemoryTrackBlock));
		pEnd->memory = nullptr;
		pEnd->size = 0;
		pEnd->prev = nullptr;

		mTrackStack[++mCurrStackPos] = { scopeName, pEnd }; // initialize end guard
	}

	void MemoryRecorder::OnExitScope()
	{
		auto* pEnd = mTrackStack[mCurrStackPos].second;
		auto* pCurr = pEnd->prev;
		while (pCurr) // check if any memory leaking happened in this scope
		{
			auto* pPreservedBlock = (MemoryTrackBlock*)mi_malloc(sizeof(MemoryTrackBlock));
			*pPreservedBlock = *pCurr; // make one copy to prevent linking issue
			Insert(pPreservedBlock, mpForgottenMemory); // preserve this memory

			auto* pBlock = pCurr;
			pCurr = pCurr->prev;
			mi_free(pBlock);
		}

		--mCurrStackPos;
		mi_free(pEnd);

		if (mCurrStackPos == -1) // escape from all the scopes
		{
			auto* pCurr = mpForgottenMemory->prev;
			while (pCurr) // dump all the leaking data
			{
				SG_LOG_WARN("Memory Leak Occurred At Scope: %s\nFile: %s : line %u\nFunction: %s \nMemory: 0x%p of %zu bytes", mTrackStack[mCurrStackPos].first,
					pCurr->file, pCurr->line, pCurr->function, pCurr->memory, pCurr->size);

				auto* pBlock = pCurr;
				pCurr = pCurr->prev;
				mi_free(pBlock);
			}
		}
	}

}