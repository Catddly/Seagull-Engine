#pragma once

#include "Core/Config.h"
#include "Common/System/IFileSystem.h"
#include "Core/FileSystem/PlatformStreamOp.h"

#include "Core/Memory/Memory.h"

namespace SG
{

	class CFileSystem final : public IFileSystem
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

		//SG_CORE_API virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut) override;
		//SG_CORE_API virtual bool Close(FileStream* pStream) override;
		//SG_CORE_API virtual Size Read(FileStream* pStream, void* pInBuf, Size bufSize) override;
		//SG_CORE_API virtual Size Write(FileStream* pStream, const void* const pOutBuf, Size bufSize) override;
		//SG_CORE_API virtual bool Seek(FileStream* pStream, EFileBaseOffset baseOffset, Size offset) override;
		//SG_CORE_API virtual Size Tell(const FileStream* pStream) override;
		//SG_CORE_API virtual Size FileSize(FileStream* pStream) override;
		//SG_CORE_API virtual bool Flush(FileStream* pStream) override;
		//SG_CORE_API virtual bool IsEndOfFile(const FileStream* pStream) override;

		SG_CORE_API virtual void SetRootDirectory(const char* filepath) override;
	private:
		// platform implementation of stream operations
		SPlatformStreamOp* mStreamOp = nullptr;
	};

}