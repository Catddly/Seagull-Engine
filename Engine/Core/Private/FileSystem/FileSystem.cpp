#include "StdAfx.h"
#include "FileSystem.h"

#include "Core/Private/Platform/Windows/WindowsStreamOp.h"
#include "Memory/IMemory.h"

#include "Stl/string.h"

namespace SG
{

	void CFileSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mStreamOp = Memory::New<SWindowsStreamOp>();
#endif
	}

	void CFileSystem::OnShutdown()
	{
		Memory::Delete(mStreamOp);
	}

	bool CFileSystem::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode)
	{
		return mStreamOp->Open(directory, filename, filemode, &mStream);
	}

	bool CFileSystem::Close()
	{
		return mStreamOp->Close(&mStream);
	}

	Size CFileSystem::Read(void* pInBuf, Size bufSize)
	{
		return mStreamOp->Read(&mStream, pInBuf, bufSize);
	}

	Size CFileSystem::Write(const void* const pOutBuf, Size bufSize)
	{
		return mStreamOp->Write(&mStream, pOutBuf, bufSize);
	}

	bool CFileSystem::Seek(EFileBaseOffset baseOffset, Size offset)
	{
		return mStreamOp->Seek(&mStream, baseOffset, offset);
	}

	Size CFileSystem::Tell() const
	{
		return mStreamOp->Tell(&mStream);
	}

	Size CFileSystem::FileSize() const
	{
		return mStreamOp->FileSize(&mStream);
	}

	bool CFileSystem::Flush()
	{
		return mStreamOp->Flush(&mStream);
	}

	bool CFileSystem::IsEndOfFile() const
	{
		return mStreamOp->IsEndOfFile(&mStream);
	}

	void CFileSystem::SetIStreamOp(IStreamOps* pStreamOp)
	{
		if (pStreamOp != nullptr)
			mStreamOp = pStreamOp;
	}

}