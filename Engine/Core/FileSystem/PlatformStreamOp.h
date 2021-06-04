#pragma once

#include "Common/System/IFileSystem.h"

namespace SG
{

	struct SPlatformStreamOp : public IStreamOp
	{
		virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut) override;
		virtual bool Close(FileStream* pStream) override;
		virtual Size Read(FileStream* pStream, void* pInBuf, Size bufSize) override;
		virtual Size Write(FileStream* pStream, const void* const pOutBuf, Size bufSize) override;
		virtual bool Seek(FileStream* pStream, EFileBaseOffset baseOffset, Size offset) override;
		virtual Size Tell(const FileStream* pStream) override;
		virtual Size FileSize(FileStream* pStream) override;
		virtual bool Flush(FileStream* pStream) override;
		virtual bool IsEndOfFile(const FileStream* pStream) override;

		// default root directory is where the .exe exist
		const char* mRootDirectory = "";
	};

}