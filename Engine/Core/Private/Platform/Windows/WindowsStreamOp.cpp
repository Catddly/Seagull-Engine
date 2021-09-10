#include "StdAfx.h"
#include "Core/Private/Platform/Windows/WindowsStreamOp.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{
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

	bool SWindowsStreamOp::Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut)
	{
		// if the EResourceDirectory folder is exist
		auto rd = (LPCSTR)gResoureceDirectory[(UInt32)directory];
		if (!::PathIsDirectoryA(rd))
			::CreateDirectoryA(rd, NULL);

		string outDirectory;
		if (directory == EResourceDirectory::eRoot)
			outDirectory += filename;
		else
		{
			outDirectory += gResoureceDirectory[(UInt32)directory];
			outDirectory += "/";
			outDirectory += filename;
		}

		int errorNo;
		switch (filemode)
		{
		case EFileMode::efRead:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "r"); break;
		case EFileMode::efRead_Binary:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "rb"); break;
		case EFileMode::efWrite:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "w"); break;
		case EFileMode::efWrite_Binary:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "wb"); break;
		case EFileMode::efAppend:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "a+"); break;
		case EFileMode::efAppend_Binary:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "a+b"); break;
		case EFileMode::efRead_Write:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "wt+"); break;
		case EFileMode::efRead_Write_Binary:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "wb+"); break;
		case EFileMode::efBinary:
			errorNo = fopen_s((FILE**)&pOut->file, outDirectory.c_str(), "wb+"); break;
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

	bool SWindowsStreamOp::Close(FileStream* pStream)
	{
		FILE* pFile = (FILE*)pStream->file;
		if (pFile)
		{
			fclose(pFile);
			return true;
		}
		else
			return false;
	}

	Size SWindowsStreamOp::Read(FileStream* pStream, void* pInBuf, Size bufSize)
	{
		return fread(pInBuf, bufSize, 1, (FILE*)pStream->file);
	}

	Size SWindowsStreamOp::Write(FileStream* pStream, const void* const pOutBuf, Size bufSize)
	{
		return fwrite(pOutBuf, bufSize, 1, (FILE*)pStream->file);
	}

	bool SWindowsStreamOp::Seek(const FileStream* pStream, EFileBaseOffset baseOffset, Size offset) const
	{
		int bOffset = baseOffset == EFileBaseOffset::eStart ? SEEK_SET :
			baseOffset == EFileBaseOffset::eCurrent ? SEEK_CUR : SEEK_END;
		int ret = fseek((FILE*)pStream->file, (long)offset, bOffset);
		if (ret == 0)
			return true;
		return false;
	}

	Size SWindowsStreamOp::Tell(const FileStream* pStream) const
	{
		return ftell((FILE*)pStream->file);
	}

	Size SWindowsStreamOp::FileSize(const FileStream* pStream) const
	{
		Size currPos = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eEOF, 0);
		Size fileSize = Tell(pStream);
		Seek(pStream, EFileBaseOffset::eStart, currPos);
		return fileSize;
	}

	bool SWindowsStreamOp::Flush(FileStream* pStream)
	{
		int ret = fflush((FILE*)pStream->file);
		if (ret == 0)
			return true;
		return false;
	}

	bool SWindowsStreamOp::IsEndOfFile(const FileStream* pStream) const
	{
		return feof((FILE*)pStream->file);
	}

}
#endif // SG_PLATFORM_WINDOWS