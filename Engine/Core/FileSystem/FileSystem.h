#pragma once

#include "Core/Config.h"
#include "Common/System/IFileSystem.h"
#include "Core/FileSystem/PlatformStreamOp.h"

namespace SG
{

	class CFileSystem final : public IFileSystem
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

		SG_CORE_API virtual void SetIStreamOp(IStreamOp* pStreamOp);

		SG_CORE_API virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode) override;
		SG_CORE_API virtual bool Close() override;
		SG_CORE_API virtual Size Read(void* pInBuf, Size bufSize) override;
		SG_CORE_API virtual Size Write(const void* const pOutBuf, Size bufSize) override;
		SG_CORE_API virtual bool Seek(EFileBaseOffset baseOffset, Size offset) override;
		SG_CORE_API virtual Size Tell() const override;
		SG_CORE_API virtual Size FileSize() const override;
		SG_CORE_API virtual bool Flush() override;
		SG_CORE_API virtual bool IsEndOfFile() const override;

		SG_CORE_API virtual void SetRootDirectory(const char* filepath) override;
	private:
		// platform implementation of stream operations
		IStreamOp* mStreamOp = nullptr;
		FileStream mStream;
	};

}