#include "StdAfx.h"
#include "ShaderComiler.h"

#include "System/System.h"
#include "System/IFileSystem.h"
#include "System/ILogger.h"
#include "Memory/IMemory.h"

#include <eastl/unique_ptr.h>

namespace SG
{

	bool ShaderCompiler::LoadSPIRVShader(const string& binShaderName, ShaderStages& outStages)
	{
		auto* pIO = SSystem()->GetFileSystem();

		UInt32 bNoFileFound = 0;
		bool bHasVert = false;
		bool bHasFrag = false;
		for (UInt32 i = 0; i < (UInt32)EShaderStages::NUM_STAGES; ++i)
		{
			string actualName = binShaderName;
			switch (i)
			{
				case 0: actualName += "-vert.spv"; bHasVert = true; break;
				case 1:	actualName += "-tesc.spv"; break;
				case 2:	actualName += "-tese.spv"; break;
				case 3:	actualName += "-geom.spv"; break;
				case 4:	actualName += "-frag.spv"; bHasFrag = true; break;
				case 5:	actualName += "-comp.spv"; break;
			}

			if (pIO->Open(EResourceDirectory::eShader_Binarires, actualName.c_str(), EFileMode::efRead_Binary))
			{
				ShaderData data;
				data.binarySize = pIO->FileSize();
				data.pBinary = (std::byte*)(Memory::Malloc(data.binarySize));
				pIO->Read(data.pBinary, pIO->FileSize());
				outStages.insert_or_assign((EShaderStages)(1 << i), data);

				pIO->Close();
			}
			else
			{
				++bNoFileFound;
				SG_LOG_WARN("Failed to find SPIRV stages: (%d)", i);
				pIO->Close();
			}
		}

		if (!bHasVert && !bHasFrag)
		{
			outStages.clear();

			SG_LOG_WARN("Necessary shader stages(vert or frag) is/are missing!");
			return false;
		}

		if (bNoFileFound == (UInt32)EShaderStages::NUM_STAGES)
		{
			SG_LOG_ERROR("No SPIRV shader is found! (%s)", binShaderName.c_str());
			return false;
		}
		return true;
	}

}