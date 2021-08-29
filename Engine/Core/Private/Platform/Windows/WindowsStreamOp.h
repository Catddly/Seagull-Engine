#pragma once

#include "System/IFileSystem.h"

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	struct SWindowsStreamOp : public IStreamOps
	{
		virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut) override;
		virtual bool Close(FileStream* pStream) override;
		virtual Size Read(FileStream* pStream, void* pInBuf, Size bufSize) override;
		virtual Size Write(FileStream* pStream, const void* const pOutBuf, Size bufSize) override;
		virtual bool Seek(const FileStream* pStream, EFileBaseOffset baseOffset, Size offset) const override;
		virtual Size Tell(const FileStream* pStream) const override;
		virtual Size FileSize(const FileStream* pStream) const override;
		virtual bool Flush(FileStream* pStream) override;
		virtual bool IsEndOfFile(const FileStream* pStream) const override;
	};

}
#endif