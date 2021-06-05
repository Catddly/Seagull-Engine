#include "StdAfx.h"
#include "Core/FileSystem/PlatformStreamOp.h"

#include "Core/Stl/string.h"
#include "Core/Stl/string_view.h"

namespace SG
{

#ifdef SG_PLATFORM_WINDOWS

	// TODO: maybe we should use a more elegant way to do the convertion
	const char* gResoureceDirectory[(UInt32)EResourceDirectory::Num_Directory] = {
		"",
		"ShaderBin",
		"ShaderSrc",
		"Mesh",
		"Texture",
		"Font",
		"Log",
		"Script"
	};

	bool SPlatformStreamOp::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut)
	{
		// if the EResourceDirectory folder is exist
		auto rd = (LPCSTR)gResoureceDirectory[(UInt32)directory];
		if (!PathIsDirectoryA(rd))
			CreateDirectoryA(rd, NULL);

		string outDirectory;
		if (mRootDirectory == "") // use default root directory
			outDirectory += gResoureceDirectory[(UInt32)directory];
		else
		{
			outDirectory += mRootDirectory;
			outDirectory += gResoureceDirectory[(UInt32)directory];
		}
		if (directory != EResourceDirectory::eRoot)
			outDirectory += "/";
		outDirectory += filename;

		int errorNo;
		switch (filemode)
		{
		case EFileMode::eRead:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "r"); break;
		case EFileMode::eRead_Binary:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "rb"); break;
		case EFileMode::eWrite:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "w"); break;
		case EFileMode::eWrite_Binary:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "wb"); break;
		case EFileMode::eAppend:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "a"); break;
		case EFileMode::eAppend_Binary:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "ab"); break;
		case EFileMode::eRead_Write:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "wt+"); break;
		case EFileMode::eRead_Write_Binary:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "wb+"); break;
		case EFileMode::eBinary:
			errorNo = fopen_s(&pOut->file, outDirectory.c_str(), "wb+"); break;
		default:
			SG_ASSERT(false && "no file mode fit!"); break;
		}

		if (errorNo != 0 || pOut->file == NULL)
		{
			return false;
		}

		pOut->filemode = filemode;
		return true;
	}

	bool SPlatformStreamOp::Close(FileStream* pStream)
	{
		FILE* pFile = pStream->file;
		if (pFile)
		{
			fclose(pFile);
			return true;
		}
		else
			return false;
	}

	Size SPlatformStreamOp::Read(FileStream* pStream, void* pInBuf, Size bufSize)
	{
		return fread(pInBuf, bufSize, 1, pStream->file);
	}

	Size SPlatformStreamOp::Write(FileStream* pStream, const void* const pOutBuf, Size bufSize)
	{
		return fwrite(pOutBuf, bufSize, 1, pStream->file);
	}

	bool SPlatformStreamOp::Seek(const FileStream* pStream, EFileBaseOffset baseOffset, Size offset) const
	{
		int bOffset = baseOffset == EFileBaseOffset::eStart ? SEEK_SET :
			baseOffset == EFileBaseOffset::eCurrent ? SEEK_CUR : SEEK_END;
		int ret = fseek(pStream->file, (long)offset, bOffset);
		if (ret == 0)
			return true;
		return false;
	}

	Size SPlatformStreamOp::Tell(const FileStream* pStream) const
	{
		return ftell(pStream->file);
	}

	Size SPlatformStreamOp::FileSize(const FileStream* pStream) const
	{
		Size currPos = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eEOF, 0);
		Size fileSize = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eStart, currPos);
		return fileSize;
	}

	bool SPlatformStreamOp::Flush(FileStream* pStream)
	{
		int ret = fflush(pStream->file);
		if (ret == 0)
			return true;
		return false;
	}

	bool SPlatformStreamOp::IsEndOfFile(const FileStream* pStream) const
	{
		return feof(pStream->file);
	}

#endif // SG_PLATFORM_WINDOWS
}