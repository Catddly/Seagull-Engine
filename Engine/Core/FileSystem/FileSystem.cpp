#include "StdAfx.h"
#include "FileSystem.h"

#include "Core/Stl/string.h"

namespace SG
{

	void CFileSystem::OnInit()
	{
		mStreamOp = New<SPlatformStreamOp>();
		// set root directory to where the .exe file is
		// TODO: move to SEnv or something control all the environment variable
		char abPath[SG_MAX_FILE_PATH] = { 0 };
		GetModuleFileNameA(NULL, abPath, sizeof(abPath));
		char drivePath[SG_MAX_DRIVE_PATH] = { 0 };
		char directoryPath[SG_MAX_DIREC_PATH] = { 0 };
		_splitpath_s(abPath, 
			drivePath, SG_MAX_DRIVE_PATH, 
			directoryPath, SG_MAX_DIREC_PATH, NULL, 0, NULL, 0);

		string rootPath(drivePath);
		rootPath.append(directoryPath);
		_chdir(rootPath.c_str());
	}

	void CFileSystem::OnShutdown()
	{
		Delete(mStreamOp);
	}

	void CFileSystem::SetRootDirectory(const char* filepath)
	{
		mStreamOp->mRootDirectory = filepath;
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

}