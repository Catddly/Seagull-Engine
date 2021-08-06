#pragma once

#include "Core/Config.h"
#include "Common/System/IFileSystem.h"

namespace SG
{

	class CFileSystem final : public IFileSystem
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

		//! Change file stream operations during runtime, can be modified by user.
		SG_CORE_API virtual void SetIStreamOp(IStreamOps* pStreamOp) override;

		SG_CORE_API virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode) override;
		SG_CORE_API virtual bool Close() override;
		SG_CORE_API virtual Size Read(void* pInBuf, Size bufSize) override;
		SG_CORE_API virtual Size Write(const void* const pOutBuf, Size bufSize) override;
		SG_CORE_API virtual bool Seek(EFileBaseOffset baseOffset, Size offset) override;
		SG_CORE_API virtual Size Tell() const override;
		SG_CORE_API virtual Size FileSize() const override;
		SG_CORE_API virtual bool Flush() override;
		SG_CORE_API virtual bool IsEndOfFile() const override;
	private:
		// implementation of stream operations
		IStreamOps* mStreamOp = nullptr;
		FileStream mStream;
	};

}