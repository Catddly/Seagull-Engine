#include "StdAfx.h"
#include "Render/ShaderComiler.h"

#include "System/System.h"
#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	bool ShaderCompiler::LoadSPIRVShader(const string& binShaderName, Shader& outStages)
	{
		UInt8 shaderBits = 0;
		if (!FileSystem::Exist(EResourceDirectory::eShader_Binarires, ""))
		{
			SG_LOG_WARN("You must put all the SPIRV binaries in the ShaderBin folder!");
			return false;
		}

		for (UInt32 i = 0; i < (UInt32)EShaderStage::NUM_STAGES; ++i)
		{
			string actualName = binShaderName;
			switch (i)
			{
			case 0: actualName += "-vert.spv"; break;
			case 1:	actualName += "-tesc.spv"; break;
			case 2:	actualName += "-tese.spv"; break;
			case 3:	actualName += "-geom.spv"; break;
			case 4:	actualName += "-frag.spv"; break;
			case 5:	actualName += "-comp.spv"; break;
			}

			if (FileSystem::Open(EResourceDirectory::eShader_Binarires, actualName.c_str(), EFileMode::efRead_Binary))
			{
				ShaderData data;
				data.binarySize = FileSystem::FileSize();
				data.pBinary = (std::byte*)(Memory::Malloc(data.binarySize));
				FileSystem::Read(data.pBinary, data.binarySize);
				outStages.insert_or_assign((EShaderStage)(1 << i), data);

				shaderBits |= (1 << i);
				FileSystem::Close();
			}
			//else
			//{
			//	SG_LOG_WARN("Failed to find SPIRV stages: (%d)", i);
			//	pIO->Close();
			//}
		}

		if ((shaderBits & (1 << 0)) == 0 || (shaderBits & (1 << 4)) == 0) // if vert or frag stage is missing
		{
			outStages.clear();

			SG_LOG_WARN("Necessary shader stages(vert or frag) is/are missing!");
			return false;
		}

		if (shaderBits == 0)
		{
			SG_LOG_ERROR("No SPIRV shader is found! (%s)", binShaderName.c_str());
			return false;
		}
		return true;
	}

	bool ShaderCompiler::CompileGLSLShader(const string& binShaderName, Shader& outStages)
	{
		char* glslc = "";
		Size num = 1;
		_dupenv_s(&glslc, &num, "VULKAN_SDK");
		string glslcPath = glslc;
		glslcPath += "\\Bin32\\glslc.exe ";

		UInt8 shaderBits = 0;
		FileSystem::ExistOrCreate(EResourceDirectory::eShader_Binarires, ""); // create ShaderBin folder if it doesn't exist

		for (UInt32 i = 0; i < (UInt32)EShaderStage::NUM_STAGES; ++i)
		{
			string extension;
			string commandLine = glslcPath;
			switch (i)
			{
			case 0: extension = ".vert"; break;
			case 1:	extension = ".tesc"; break;
			case 2:	extension = ".tese"; break;
			case 3:	extension = ".geom"; break;
			case 4:	extension = ".frag"; break;
			case 5:	extension = ".comp"; break;
			}
			string actualName = binShaderName + extension;
			string compiledName = actualName;
			compiledName[actualName.find('.')] = '-';
			compiledName += ".spv";

			if (FileSystem::Exist(EResourceDirectory::eShader_Binarires, compiledName.c_str())) // already compiled this shader once, skip it.
			{
				shaderBits |= (1 << i); // mark as exist.
				continue;
			}

			if (FileSystem::Exist(EResourceDirectory::eShader_Sources, actualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
			{
				string shaderPath = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Sources, SG_ENGINE_DEBUG_BASE_OFFSET);
				shaderPath += actualName;
				string outputPath = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + compiledName;

				commandLine += shaderPath;
				commandLine += " -o ";
				commandLine += outputPath;

				const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
				string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + binShaderName + "-" +
					extension.substr(1, extension.size() - 1) + "-compile.log";

				// create a process to use vulkanSDK to compile shader sources to binary (spirv)
				if (SSystem()->RunProcess(commandLine, pOut.c_str()) != 0)
				{
					SG_LOG_WARN("%s", pOut);
					SG_ASSERT(false);
				}

				// record what shader stage we had compiled
				shaderBits |= (1 << i);
			}
			//else
			//{
			//	SG_LOG_WARN("Failed to find GLSL shader stage: (%d)", i);
			//}
		}

		if (shaderBits == 0)
			return false;

		return LoadSPIRVShader(binShaderName, outStages);;
	}

}