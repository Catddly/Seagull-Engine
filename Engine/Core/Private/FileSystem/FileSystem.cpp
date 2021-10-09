#include "StdAfx.h"
#include "FileSystem.h"

#include "Core/Private/Platform/Windows/WindowsStreamOp.h"
#include "Memory/IMemory.h"

#include "stl/string.h"

namespace SG
{

	const char* FileSystem::sResourceDirectory[9] = {
		"",
		"ShaderBin",
		"ShaderSrc",
		"Mesh",
		"Texture",
		"Font",
		"Log",
		"Script",
		""
	};

	void FileSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mStreamOp = Memory::New<SWindowsStreamOp>();
#endif
	}

	void FileSystem::OnShutdown()
	{
		Memory::Delete(mStreamOp);
	}

	bool FileSystem::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode)
	{
		return mStreamOp->Open(directory, filename, filemode, &mStream);
	}

	bool FileSystem::Close()
	{
		return mStreamOp->Close(&mStream);
	}

	Size FileSystem::Read(void* pInBuf, Size bufSizeInByte)
	{
		return mStreamOp->Read(&mStream, pInBuf, bufSizeInByte);
	}

	Size FileSystem::Write(const void* const pOutBuf, Size bufSizeInByte)
	{
		return mStreamOp->Write(&mStream, pOutBuf, bufSizeInByte);
	}

	bool FileSystem::Seek(EFileBaseOffset baseOffset, Size offset)
	{
		return mStreamOp->Seek(&mStream, baseOffset, offset);
	}

	Size FileSystem::Tell() const
	{
		return mStreamOp->Tell(&mStream);
	}

	Size FileSystem::FileSize() const
	{
		return mStreamOp->FileSize(&mStream);
	}

	bool FileSystem::Flush()
	{
		return mStreamOp->Flush(&mStream);
	}

	bool FileSystem::IsEndOfFile() const
	{
		return mStreamOp->IsEndOfFile(&mStream);
	}

	void FileSystem::SetIStreamOp(IStreamOps* pStreamOp)
	{
		if (pStreamOp != nullptr)
			mStreamOp = pStreamOp;
	}

	bool FileSystem::Exist(const EResourceDirectory directory, const char* filename)
	{
		string filepath = sResourceDirectory[(UInt32)directory];
		filepath += "/";
		filepath += filename;
		if (_access(filepath.c_str(), 0) == 0)
			return true;
		else
			return false;
	}

}