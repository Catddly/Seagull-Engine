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
		enum {
			SG_MAX_FILE_PATH = 200,
			SG_MAX_DRIVE_PATH = 5,
			SG_MAX_FILE_NAME = 30,
			SG_MAX_EXT_PATH = 10,
			SG_MAX_DIREC_PATH = SG_MAX_FILE_PATH - SG_MAX_DRIVE_PATH - SG_MAX_FILE_NAME - SG_MAX_EXT_PATH,
		};

		SG_CORE_API virtual void OnInit() override;
		SG_CORE_API virtual void OnShutdown() override;

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
		SPlatformStreamOp* mStreamOp = nullptr;
		FileStream mStream;
	};

}