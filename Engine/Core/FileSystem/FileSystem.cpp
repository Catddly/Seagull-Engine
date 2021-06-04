#include "StdAfx.h"
#include "FileSystem.h"

namespace SG
{

	void CFileSystem::OnInit()
	{
		mStreamOp = New<SPlatformStreamOp>();
	}

	void CFileSystem::OnShutdown()
	{
		Delete(mStreamOp);
	}

	void CFileSystem::SetRootDirectory(const char* filepath)
	{
		mStreamOp->mRootDirectory = filepath;
	}

}