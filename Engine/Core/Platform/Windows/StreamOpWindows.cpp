#include "StdAfx.h"
#include "Core/FileSystem/PlatformStreamOp.h"

#include "Core/Stl/string.h"
#include "Core/Stl/string_view.h"

#include <stdio.h>

namespace SG
{

#ifdef SG_PLATFORM_WINDOWS

	// TODO: maybe we should use a more elegant way to do the convertion
	const char* gResoureceDirectory[(UInt32)EResourceDirectory::Num_Directory] = {
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
		string outDirectory;
		if (mRootDirectory == "") // use default root directory
			outDirectory += gResoureceDirectory[(UInt32)directory];
		else
		{
			outDirectory += mRootDirectory;
			outDirectory += gResoureceDirectory[(UInt32)directory];
		}
		outDirectory += "/";
		outDirectory += filename;

		FILE* pFile = pOut->file;
		switch (filemode)
		{
		case EFileMode::eRead:
			pFile = fopen(outDirectory.c_str(), "r"); break;
		case EFileMode::eRead_Binary:
			pFile = fopen(outDirectory.c_str(), "rb"); break;
		case EFileMode::eWrite:
			pFile = fopen(outDirectory.c_str(), "w"); break;
		case EFileMode::eWrite_Binary:
			pFile = fopen(outDirectory.c_str(), "wb"); break;
		case EFileMode::eAppend:
			pFile = fopen(outDirectory.c_str(), "a"); break;
		case EFileMode::eAppend_Binary:
			pFile = fopen(outDirectory.c_str(), "ab"); break;
		case EFileMode::eRead_Write:
			pFile = fopen(outDirectory.c_str(), "r+"); break;
		case EFileMode::eRead_Write_Binary:
			pFile = fopen(outDirectory.c_str(), "r+b"); break;
		case EFileMode::eBinary:
			pFile = fopen(outDirectory.c_str(), "r+b"); break;
		default:
			SG_ASSERT(false && "no file mode fit!");
			break;
		}

		if (pFile == NULL)
			return false;

		pOut->filemode = filemode;
		pOut->size = FileSize(pOut);
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

	bool SPlatformStreamOp::Seek(FileStream* pStream, EFileBaseOffset baseOffset, Size offset)
	{
		int bOffset = baseOffset == EFileBaseOffset::eStart ? SEEK_SET :
			baseOffset == EFileBaseOffset::eCurrent ? SEEK_CUR : SEEK_END;
		int ret = fseek(pStream->file, offset, bOffset);
		if (ret == 0)
			return true;
		return false;
	}

	Size SPlatformStreamOp::Tell(const FileStream* pStream)
	{
		return ftell(pStream->file);
	}

	Size SPlatformStreamOp::FileSize(FileStream* pStream)
	{
		Size currPos = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eEOF, 0);
		Size fileSize = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eStart, currPos);
		pStream->size = fileSize;
		return fileSize;
	}

	bool SPlatformStreamOp::Flush(FileStream* pStream)
	{
		int ret = fflush(pStream->file);
		if (ret == 0)
			return true;
		return false;
	}

	bool SPlatformStreamOp::IsEndOfFile(const FileStream* pStream)
	{
		return feof(pStream->file);
	}

#endif // SG_PLATFORM_WINDOWS
}