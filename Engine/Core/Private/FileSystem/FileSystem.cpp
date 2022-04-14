#include "StdAfx.h"
#include "System/FileSystem.h"

#ifdef SG_PLATFORM_WINDOWS
#	include "Core/Private/Platform/Windows/StreamOp_Windows.h"
#	include "Core/Private/Platform/Windows/FileInfo_Windows.h"
#endif
#include "Memory/Memory.h"

#include "stl/string.h"
#include "eastl/queue.h"
#include <filesystem>

namespace SG
{

	const char* FileSystem::sResourceDirectory[(UInt32)EResourceDirectory::Num_Directory + 1] = {
		"",
		"ShaderBin",
		"ShaderSrc",
		"Mesh",
		"Texture",
		"Font",
		"Log",
		"Script",
		"Vendor",
		""
	};

	IStreamOps* FileSystem::mpStreamOp = nullptr;
	FileStream* FileSystem::mpCurrentStream = nullptr;
	FileStream  FileSystem::mStream;

	void FileSystem::OnInit()
	{
#ifdef SG_PLATFORM_WINDOWS
		mpStreamOp = Memory::New<WindowsStreamOp>();
#endif
		if (!mpStreamOp)
			SG_ASSERT(false);
		SetToDefaultFileStream();
	}

	void FileSystem::OnShutdown()
	{
		Memory::Delete(mpStreamOp);
	}

	bool FileSystem::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, Size rootFolderOffset)
	{
		return mpStreamOp->Open(directory, filename, filemode, mpCurrentStream, rootFolderOffset);
	}

	bool FileSystem::Close()
	{
		return mpStreamOp->Close(mpCurrentStream);
	}

	Size FileSystem::Read(void* pInBuf, Size bufSizeInByte)
	{
		return mpStreamOp->Read(mpCurrentStream, pInBuf, bufSizeInByte);
	}

	Size FileSystem::Write(const void* const pOutBuf, Size bufSizeInByte)
	{
		return mpStreamOp->Write(mpCurrentStream, pOutBuf, bufSizeInByte);
	}

	bool FileSystem::Seek(EFileBaseOffset baseOffset, Size offset)
	{
		return mpStreamOp->Seek(mpCurrentStream, baseOffset, offset);
	}

	Size FileSystem::Tell()
	{
		return mpStreamOp->Tell(mpCurrentStream);
	}

	Size FileSystem::FileSize()
	{
		return mpStreamOp->FileSize(mpCurrentStream);
	}

	Size FileSystem::FileSize(const EResourceDirectory directory, const string& filename)
	{
		string file = GetResourceFolderPath(directory);
		file += filename;
		return std::filesystem::file_size(file.c_str());
	}

	bool FileSystem::RemoveFile(const EResourceDirectory directory, const string& filename)
	{
		string file = GetResourceFolderPath(directory);
		file += filename;
		return std::filesystem::remove(file.c_str());
	}

	bool FileSystem::Flush()
	{
		return mpStreamOp->Flush(&mStream);
	}

	bool FileSystem::IsEndOfFile()
	{
		return mpStreamOp->IsEndOfFile(&mStream);
	}

	string FileSystem::GetResourceFolderPath(EResourceDirectory directory, UInt32 baseOffset)
	{
		string path = "";
		if (baseOffset != 0)
		{
			for (UInt32 i = 0; i < baseOffset; ++i)
				path += "../";
			path += "Resources/";
		}

		path += GetResourceFolderName(directory);
		path += "/";
		return eastl::move(path);
	}

	string FileSystem::GetResourceFolderName(EResourceDirectory directory)
	{
		if (directory == EResourceDirectory::Num_Directory)
			SG_ASSERT(false && "Invalid directory");
		return sResourceDirectory[(UInt32)directory];
	}

	string FileSystem::ToAbsolutePath(const string& path)
	{
		return std::filesystem::absolute(path.c_str()).generic_string().c_str();
	}

	string FileSystem::ToRelativePath(const string& path)
	{
		return std::filesystem::relative(path.c_str()).generic_string().c_str();
	}

	void FileSystem::TraverseFileAndSubDirectoryInFolder(EResourceDirectory directory, const string& folderPath, FileTraverseFunc func, UInt32 baseOffset)
	{
		string folder = GetResourceFolderPath(directory, baseOffset);
		folder += folderPath;

		if (Exist(directory, folderPath.c_str(), baseOffset))
		{
			eastl::queue<std::filesystem::directory_iterator> iterators;
			iterators.emplace(folder.c_str());

			while (!iterators.empty())
			{
				const auto iterator = iterators.front();
				iterators.pop();

				for (const auto& fileEntry : iterator)
				{
					if (fileEntry.is_directory())
						iterators.emplace(fileEntry);
					else
					{
						string filename = fileEntry.path().generic_string().c_str();
						string folderPath = GetResourceFolderName(directory);
						Size splitPos = filename.find(folderPath);
						func(filename.substr(splitPos + folderPath.size() + 1, filename.size() - splitPos).c_str());
					}
				}
			}
		}
	}

	void FileSystem::TraverseFilesInFolder(EResourceDirectory directory, const string& folderPath, FileTraverseFunc func, UInt32 baseOffset)
	{
		string folder = GetResourceFolderPath(directory, baseOffset);
		folder += folderPath;

		if (Exist(directory, folderPath.c_str(), baseOffset))
		{
			std::filesystem::directory_iterator iterator{ folderPath.c_str() };
			for (const auto& fileEntry : iterator)
				func(fileEntry.path().filename().generic_string().c_str());
		}
	}

	void FileSystem::SetIStreamOp(IStreamOps* pStreamOp)
	{
		if (pStreamOp != nullptr)
			mpStreamOp = pStreamOp;
	}

	void FileSystem::SetFileStream(FileStream* pFileStream)
	{
		mpCurrentStream = pFileStream;
	}

	void FileSystem::SetToDefaultFileStream()
	{
		mpCurrentStream = &mStream;
	}

	bool FileSystem::Exist(const EResourceDirectory directory, const char* filename, UInt32 baseOffset)
	{
		string filepath = GetResourceFolderPath(directory, baseOffset);
		filepath += filename;
		return std::filesystem::exists(filepath.c_str());
	}

	bool FileSystem::CreateFolder(const EResourceDirectory directory, const char* folderName)
	{
		string path = GetResourceFolderName(directory) + "/";
		path += folderName;
		return std::filesystem::create_directory(path.c_str());
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
				if (path.size() != 0) // path still have a folder to create
					folder += path;

				if (!Exist(directory, folder.c_str()))
					bSuccess &= CreateFolder(directory, folder.c_str());
				break;
			}
			else
			{
				if (path[nextFolderPos + 1] == '/') // if use // or /
					++nextFolderPos;
				++nextFolderPos;
				folder += path.substr(0, nextFolderPos);
				path = path.substr(nextFolderPos, path.size() - nextFolderPos);
				if (!Exist(directory, folder.c_str()))
					bSuccess &= CreateFolder(directory, folder.c_str());
			}
		}
		return bSuccess;
	}

	TimePoint FileSystem::GetFileCreateTime(EResourceDirectory directory, const char* filename, UInt32 baseOffset)
	{
		string path = GetResourceFolderPath(directory, baseOffset);
		path += filename;
#ifdef SG_PLATFORM_WINDOWS
		return SG::GetFileCreateTime(path.c_str());
#endif
	}

	TimePoint FileSystem::GetFileLastWriteTime(EResourceDirectory directory, const char* filename, UInt32 baseOffset)
	{
		string path = GetResourceFolderPath(directory, baseOffset);
		path += filename;
#ifdef SG_PLATFORM_WINDOWS
		return SG::GetFileLastWriteTime(path.c_str());
#endif
	}

	TimePoint FileSystem::GetFileLastReadTime(EResourceDirectory directory, const char* filename, UInt32 baseOffset)
	{
		string path = GetResourceFolderPath(directory, baseOffset);
		path += filename;
#ifdef SG_PLATFORM_WINDOWS
		return SG::GetFileLastReadTime(path.c_str());
#endif
	}


}