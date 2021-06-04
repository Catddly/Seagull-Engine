#pragma once

//#include "Common/Base/BasicTypes.h"
#include "../Base/BasicTypes.h"

#include <stdio.h>

namespace SG
{

#ifdef __cplusplus
extern "C"
{
#endif

	//! Set engine's resource files root directory.
	//! Default root directory will be the folder where .exe is in.
	//! @param (filepath) relative path to the .exe
	void SetRootDirectory(const char* filepath);

	//! Resource directory base on the root directory.
	typedef enum class EResourceDirectory
	{
		eShader_Binarires = 0,
		eShader_Sources,
		eMeshes,
		eTextures,
		eFonts,
		eLog,
		eScripts,
		Num_Directory,
	} EResourceDirectory;

	//! Resourece filters to define the usage of files.
	typedef enum class EResoureceFilter
	{
		eGame_Resoureces = 0, //! Resources used in game
		eGame_Savedatas,      //! Savedatas for game
		eGame_Scripts,        //! Scripts used in game
		eDebug,               //! During development, for debug purpose
	} EResoureceFilter;

	typedef enum class EFileMode
	{
		eRead = 0x01,
		eWrite = 0x02,
		eAppend = 0x04,
		eBinary = 0x08,

		eRead_Write = eRead | eWrite,
		eRead_Binary = eRead | eBinary,
		eWrite_Binary = eWrite | eBinary,
		eAppend_Binary = eAppend | eBinary,
		eRead_Write_Binary = eRead_Write | eBinary,
	} EFileMode;
	SG_ENUM_CLASS_FLAG(UInt32, EFileMode);

	//! Memory stream for binary
	typedef struct Memory
	{
		UInt8* pBuffer;
		Size   cursor;
	} Memory;

	//! C file wrapper
	typedef struct File
	{
		FILE*  pFile;
		Size   cursor;
	} File;

	typedef struct FileStream
	{
		union
		{
			Memory memory;
			File   file;
		};
		IntPtr    size;
		EFileMode filemode;
	};

	//! Stream operations to manipulate the files in the disk
	typedef struct IStreamOp
	{
		bool (*open)(const EResourceDirectory directory, const char* filename, const EFileMode filemode, FileStream* pOut);
		bool (*close)(FileStream* pStream);
		Size (*read)(FileStream* pStream, void* pInBuf, Size bufSize);
		Size (*write)(FileStream* pStream, const void* const pOutBuf, Size bufSize);
	} IStreamOp;

#ifdef __cplusplus
} // end extern C
#endif

}