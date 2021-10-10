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
		mStreamOp = Memory::New<WindowsStreamOp>();
#endif
	}

	void FileSystem::OnShutdown()
	{
		Memory::Delete(mStreamOp);
	}

	bool FileSystem::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, Size rootFolderOffset)
	{
		return mStreamOp->Open(directory, filename, filemode, &mStream, rootFolderOffset);
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

	bool FileSystem::Exist(const EResourceDirectory directory, const char* filename, const char* prefix)
	{
		string filepath = prefix;
		filepath += sResourceDirectory[(UInt32)directory];
		filepath += "//";
		filepath += filename;
		if (_access(filepath.c_str(), 0) == 0)
			return true;
		else
			return false;
	}

	bool FileSystem::CreateFolder(const EResourceDirectory directory, const char* folderName)
	{
		string path = sResourceDirectory[(UInt32)directory];
		path += "//";
		path += folderName;
		if (_mkdir(path.c_str()) == 0)
			return true;
		return false;
	}

	bool FileSystem::ExistOrCreate(const EResourceDirectory directory, const string& filename)
	{
		bool bSuccess = true;
		string folder = "";
		string path = filename;
		while (true)
		{
			Size nextFolderPos = path.find_first_of('/');
			if (nextFolderPos == string::npos) // no folder path in the filename
			{
				if (!Exist(directory, folder.c_str()))
					bSuccess &= CreateFolder(directory, folder.c_str());
				break;
			}
			else
			{
				folder += path.substr(0, nextFolderPos - 1);
				if (path[nextFolderPos + 1] == '/') // if use // or /
					++nextFolderPos;
				path = path.substr(nextFolderPos, path.size() - nextFolderPos);
				if (!Exist(directory, folder.c_str()))
					bSuccess &= CreateFolder(directory, folder.c_str());
			}
		}
		return bSuccess;
	}

}