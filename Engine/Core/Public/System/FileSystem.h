#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

#include "stl/string.h"

namespace SG
{

#define SG_ENGINE_DEBUG_BASE_OFFSET 3

	//! Resource directory base on the root directory.
	enum class EResourceDirectory
	{
		eRoot = 0,
		eShader_Binarires,
		eShader_Sources,
		eMeshes,
		eTextures,
		eFonts,
		eLog,
		eScripts,
		Num_Directory,
	};

	//! Resource filters to define the usage of files.
	enum class EResoureceFilter
	{
		eGame_Resoureces = 0, //! Resources used in game
		eGame_Savedatas,      //! Savedatas for game
		eGame_Scripts,        //! Scripts used in game
		eDebug,               //! During development, for debug purpose
	};

	enum class EFileBaseOffset
	{
		eStart,
		eCurrent,
		eEOF,
	};

	enum class EFileMode : UInt32
	{
		efRead = 0x01,
		efWrite = 0x02,
		efAppend = 0x04,
		efBinary = 0x08,

		efRead_Write = efRead | efWrite,
		efRead_Binary = efRead | efBinary,
		efWrite_Binary = efWrite | efBinary,
		efAppend_Binary = efAppend | efBinary,
		efRead_Write_Binary = efRead_Write | efBinary,
		efUndefined = 0xff,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EFileMode);

	//! Memory stream for binary
	struct MemoryBlock
	{
		UInt8* pBuffer;
		Size   cursor;
	};

	struct FileStream
	{
		union
		{
			MemoryBlock memory;
			void*       file;
		};
		EFileMode  filemode;
	};

	//! Stream operations to manipulate the files in the disk
	interface IStreamOps
	{
		virtual ~IStreamOps() = default;

		virtual bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut, Size rootFolderOffset = 0) = 0;
		virtual bool Close(FileStream* pStream) = 0;
		virtual Size Read(FileStream* pStream, void* pInBuf, Size bufSize) = 0;
		virtual Size Write(FileStream* pStream, const void* const pOutBuf, Size bufSize) = 0;
		virtual bool Seek(const FileStream* pStream, EFileBaseOffset baseOffset, Size offset) const = 0;
		virtual Size Tell(const FileStream* pStream) const = 0;
		virtual Size FileSize(const FileStream* pStream) const = 0;
		virtual bool Flush(FileStream* pStream) = 0;
		virtual bool IsEndOfFile(const FileStream* pStream) const = 0;
	};

	class FileSystem
	{
	public:
		//! Change file stream operations during runtime, can be modified by user.
		SG_CORE_API static void SetIStreamOp(IStreamOps* pStreamOp);

		SG_CORE_API static bool Exist(const EResourceDirectory directory, const char* filename, UInt32 baseOffset = 0);
		SG_CORE_API static bool ExistOrCreate(const EResourceDirectory directory, const string& filename);

		SG_CORE_API static bool Open(const EResourceDirectory directory, const char* filename, const EFileMode filemode, Size rootFolderOffset = 0);
		SG_CORE_API static bool Close();
		SG_CORE_API static Size Read(void* pInBuf, Size bufSizeInByte);
		SG_CORE_API static Size Write(const void* const pOutBuf, Size bufSizeInByte);
		SG_CORE_API static bool Seek(EFileBaseOffset baseOffset, Size offset);
		SG_CORE_API static Size Tell();
		SG_CORE_API static Size FileSize();
		SG_CORE_API static bool Flush();
		SG_CORE_API static bool IsEndOfFile();

		SG_CORE_API static string GetResourceFolderPath(EResourceDirectory directory, UInt32 baseOffset = 0);

		SG_CORE_API static bool CreateFolder(const EResourceDirectory directory, const char* folderName);
	private:
		friend class System;

		static void OnInit();
		static void OnShutdown();
	private:
#ifdef SG_PLATFORM_WINDOWS
		friend struct WindowsStreamOp;
#endif
		// implementation of stream operations
		static IStreamOps* mStreamOp;
		static FileStream  mStream;

		static const char* sResourceDirectory[9];
	};

}