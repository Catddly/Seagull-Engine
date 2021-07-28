#include "StdAfx.h"
#include "ShaderVk.h"

#include "Core/System/System.h"
#include "Common/System/IFileSystem.h"

#include "Common/Memory/IMemory.h"

#include "Common/Stl/string.h"

namespace SG
{

	ShaderVk::ShaderVk(Renderer* pRenderer, string_view filepath)
		:mpRenderer(pRenderer), mShaderLanguage(EShaderLanguage::eGLSL)
	{
		Size dotPos = filepath.find_last_of('.');
		string_view name = filepath.substr(0, dotPos);
		string_view extension = filepath.substr(dotPos + 1, filepath.size() - dotPos);

		auto* pFS = System::GetInstance()->GetIFileSystem();
		if (extension == "spv")
		{
			if (pFS->Open(EResourceDirectory::eShader_Binarires, string(filepath).c_str(), EFileMode::eRead_Binary)) // convert to string to ensure the null-terminated string
			{
				Size fileSize = pFS->FileSize();
				pFS->Read(&mBinary, fileSize);
				pFS->Close();
			}
		}
		else if (extension == "vert" || extension == "frag" || extension == "comp" || extension == "geom")
		{
			// prepare for compile
			string glslangValidator = ::getenv("VULKAN_SDK");
			glslangValidator += "/Bin32/glslc.exe";
			string shaderPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Sources) + string(filepath);
			string outputPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + string(name) + "-" + string(extension) + ".spv";

			const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
			const char* pOut = "";

			// create a process to use vulkanSDK to compile shader sources to binary (spirv)
			System::GetInstance()->RunProcess(glslangValidator.c_str(), args, 3, pOut);
		}
		else
		{
			SG_LOG_WARN("Unknown file type of shader!");
			SG_ASSERT(false);
		}
	}

	ShaderVk::~ShaderVk()
	{
		if (mBinary)
			delete mBinary;
	}

}