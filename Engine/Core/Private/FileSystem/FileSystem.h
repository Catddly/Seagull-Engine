#pragma once

#include "Core/Config.h"
#include "System/IFileSystem.h"

namespace SG
{

	class FileSystem final : public IFileSystem
	{
	public:
		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

		//! Change file stream operations during runtime, can be modified by user.
		SG_CORE_API virtual void SetIStreamOp(IStreamOps* pStreamOp) override;

		SG_CORE_API virtual bool Exist(const EResourceDirectory directory, const char* filename, const char* prefix = "") override;
		SG_CORE_API virtual bool ExistOrCreate(const EResourceDirectory directory, const string& filename) override;

		SG_CORE_API virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, Size rootFolderOffset = 0) override;
		SG_CORE_API virtual bool Close() override;
		SG_CORE_API virtual Size Read(void* pInBuf, Size bufSizeInByte) override;
		SG_CORE_API virtual Size Write(const void* const pOutBuf, Size bufSizeInByte) override;
		SG_CORE_API virtual bool Seek(EFileBaseOffset baseOffset, Size offset) override;
		SG_CORE_API virtual Size Tell() const override;
		SG_CORE_API virtual Size FileSize() const override;
		SG_CORE_API virtual bool Flush() override;
		SG_CORE_API virtual bool IsEndOfFile() const override;

		SG_CORE_API virtual bool CreateFolder(const EResourceDirectory directory, const char* folderName) override;

		SG_CORE_API virtual const char* GetRegisterName() const override { return "FileSystem"; }
	private:
#ifdef SG_PLATFORM_WINDOWS
		friend struct WindowsStreamOp;
#endif
		// implementation of stream operations
		IStreamOps* mStreamOp = nullptr;
		FileStream  mStream;

		static const char* sResourceDirectory[9];
	};

}