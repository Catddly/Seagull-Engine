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

			ReadInShaderData(actualName, i, outStages, shaderBits);
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

	bool ShaderCompiler::LoadSPIRVShader(const string& vertShaderName, const string& fragShaderName, Shader& outStages)
	{
		UInt8 shaderBits = 0;
		if (!FileSystem::Exist(EResourceDirectory::eShader_Binarires, ""))
		{
			SG_LOG_WARN("You must put all the SPIRV binaries in the ShaderBin folder!");
			return false;
		}

		string vertName = vertShaderName + "-vert.spv";
		string fragName = fragShaderName + "-frag.spv";

		ReadInShaderData(vertName, 0, outStages, shaderBits);
		ReadInShaderData(fragName, 4, outStages, shaderBits);

		if ((shaderBits & (1 << 0)) == 0 || (shaderBits & (1 << 4)) == 0) // if vert or frag stage is missing
		{
			outStages.clear();

			SG_LOG_WARN("Necessary shader stages(vert or frag) is/are missing!");
			return false;
		}

		if (shaderBits == 0)
		{
			SG_LOG_ERROR("No SPIRV shader is found! (vert: %s, frag: %s)", vertShaderName.c_str(), fragShaderName.c_str());
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
				string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + binShaderName + "-" +
					extension.substr(1, extension.size() - 1) + "-compile.log";

				if (CompileShaderVkSDK(actualName, compiledName, commandLine, pOut))
					shaderBits |= (1 << i); // record what shader stage we had compiled
				else
					SG_LOG_ERROR("Failed to compile shader: %s", actualName.c_str());
			}
			//else
			//{
			//	SG_LOG_WARN("Failed to find GLSL shader stage: (%d)", i);
			//}
		}

		if (shaderBits == 0)
			return false;

		return LoadSPIRVShader(binShaderName, outStages);
	}

	bool ShaderCompiler::CompileGLSLShader(const string& vertShaderName, const string& fragShaderName, Shader& outStages)
	{
		char* glslc = "";
		Size num = 1;
		_dupenv_s(&glslc, &num, "VULKAN_SDK");
		string glslcPath = glslc;
		glslcPath += "\\Bin32\\glslc.exe ";

		UInt8 shaderBits = 0;
		FileSystem::ExistOrCreate(EResourceDirectory::eShader_Binarires, ""); // create ShaderBin folder if it doesn't exist

		{
			string commandLine = glslcPath;

			string vertActualName = vertShaderName + ".vert";
			string vertCompiledName = vertActualName;
			vertCompiledName[vertActualName.find('.')] = '-';
			vertCompiledName += ".spv";

			if (FileSystem::Exist(EResourceDirectory::eShader_Binarires, vertCompiledName.c_str())) // already compiled this shader once, skip it.
				shaderBits |= (1 << 0); // mark as exist.

			if (FileSystem::Exist(EResourceDirectory::eShader_Sources, vertActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
			{
				string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + vertShaderName + "-vert-compile.log";

				if (CompileShaderVkSDK(vertActualName, vertCompiledName, commandLine, pOut))
					shaderBits |= (1 << 0); // record what shader stage we had compiled
				else
					SG_LOG_ERROR("Failed to compile vertex shader: %s", vertActualName.c_str());
			}
		}

		{
			string commandLine = glslcPath;

			string fragActualName = fragShaderName + ".frag";
			string fragCompiledName = fragActualName;
			fragCompiledName[fragActualName.find('.')] = '-';
			fragCompiledName += ".spv";

			if (FileSystem::Exist(EResourceDirectory::eShader_Binarires, fragCompiledName.c_str())) // already compiled this shader once, skip it.
				shaderBits |= (1 << 4); // mark as exist.

			if (FileSystem::Exist(EResourceDirectory::eShader_Sources, fragActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
			{
				string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + fragShaderName + "-frag-compile.log";

				if (CompileShaderVkSDK(fragActualName, fragCompiledName, commandLine, pOut))
					shaderBits |= (1 << 4); // record what shader stage we had compiled
				else
					SG_LOG_ERROR("Failed to compile fragment shader: %s", fragActualName.c_str());
			}
		}

		if (shaderBits == 0)
			return false;

		return LoadSPIRVShader(vertShaderName, fragShaderName, outStages);
	}

	void ShaderCompiler::ReadInShaderData(const string& name, UInt32 stage, Shader& shader, UInt8& checkFlag)
	{
		if (FileSystem::Open(EResourceDirectory::eShader_Binarires, name.c_str(), EFileMode::efRead_Binary))
		{
			ShaderData shaderData;
			shaderData.binarySize = FileSystem::FileSize();
			shaderData.pBinary = (std::byte*)(Memory::Malloc(shaderData.binarySize));
			FileSystem::Read(shaderData.pBinary, shaderData.binarySize);
			shader.insert_or_assign((EShaderStage)(1 << stage), shaderData);

			checkFlag |= (1 << stage);
			FileSystem::Close();
		}
	}

	bool ShaderCompiler::CompileShaderVkSDK(const string& actualName, const string& compiledName, string& exePath, const string& pOut)
	{
		string shaderPath = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Sources, SG_ENGINE_DEBUG_BASE_OFFSET);
		shaderPath += actualName;
		string outputPath = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + compiledName;

		exePath += shaderPath;
		exePath += " -o ";
		exePath += outputPath;

		const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };

		// create a process to use vulkanSDK to compile shader sources to binary (spirv)
		if (SSystem()->RunProcess(exePath, pOut.c_str()) != 0)
		{
			SG_LOG_WARN("%s", pOut);
			return false;
		}

		return true;
	}

}