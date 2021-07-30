#include "StdAfx.h"
#include "ShaderVk.h"

#include "Core/System/System.h"
#include "Common/System/IFileSystem.h"

#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "Common/Memory/IMemory.h"

#include <EASTL/algorithm.h>

namespace SG
{

	ShaderVk::ShaderVk(Renderer* pRenderer, const string* shaderStages, Size numShaderStages)
		:mpRenderer(pRenderer), mShaderLanguage(EShaderLanguage::eGLSL)
	{
		for (Size i = 0; i < numShaderStages; i++)
		{
			const string& stage = shaderStages[i];

			Size dotPos = stage.find_last_of('.');
			string name = stage.substr(0, dotPos);
			string extension = stage.substr(dotPos + 1, stage.size() - dotPos);

			auto* pFS = System::GetInstance()->GetIFileSystem();
			if (extension == "spv")
			{
				if (!ReadBinaryFromDisk(name, extension)) // no spirv file exist, try to compile the file from ShaderSrc
				{
					Size slashPos = stage.find_last_of('-');
					if (slashPos == string::npos)
					{
						SG_LOG_ERROR("Invalid spv file format. (spv format: shader-vert.spv)");
						SG_ASSERT(false);
					}
					string compiledPath = stage.substr(0, dotPos);  // convert to string to ensure the null-terminated string
					compiledPath[slashPos] = '.';
					string name = compiledPath.substr(0, slashPos);
					string extension = compiledPath.substr(slashPos + 1, compiledPath.size() - slashPos);
					CompileFromVulkanSDK(name, extension);
					if (!ReadBinaryFromDisk(name, extension))
					{
						SG_LOG_ERROR("No such file exists! (%s)", stage.c_str());
						// TODO: no assert, but throw an error.
						SG_ASSERT(false);
					}
				}
			}
			else if (extension == "vert" || extension == "frag" || extension == "comp" || extension == "geom" || extension == "tesc" || extension == "tese") // compiled the source shader to spirv
			{
				if (!ReadBinaryFromDisk(name, extension)) // no spirv file exist, try to compile the file from ShaderSrc
				{
					CompileFromVulkanSDK(name, extension);
					if (!ReadBinaryFromDisk(name, extension))
					{
						SG_LOG_ERROR("No such file exists! (%s.%s)", name, extension);
						SG_ASSERT(false);
					}
				}
			}
			else
			{
				SG_LOG_WARN("Unknown file type of shader!");
				SG_ASSERT(false);
			}

			if (extension == "spv")
			{
				Size slashPos = name.find_last_of('-');
				extension = name.substr(slashPos + 1, name.size() - slashPos);
			}
			if (extension == "vert")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eVert]);
			else if (extension == "frag")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eFrag]);
			else if (extension == "comp")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eComp]);
			else if (extension == "gemo")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eGeom]);
			else if (extension == "tesc")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eTesc]);
			else if (extension == "tese")
				CreateVulkanShaderModule(&mShaderStages[EShaderStages::eTese]);
		}
	}

	ShaderVk::~ShaderVk()
	{
		for (auto beg = mShaderStages.begin(); beg != mShaderStages.end(); beg++)
		{
			Free(beg->second.pBinary);
			VkShaderModule shaderModule = (VkShaderModule)beg->second.pShader;
			vkDestroyShaderModule((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), shaderModule, nullptr);
		}
	}

	SG::ShaderStages* ShaderVk::GetShaderStages()
	{
		return &mShaderStages;
	}

	string ShaderVk::CompileFromVulkanSDK(const string& name, const string& extension) const
	{
		char* glslc = "";
		Size num = 1;
		_dupenv_s(&glslc, &num, "VULKAN_SDK");
		glslc[num + 1] = '\0';
		strcat_s(glslc, sizeof(wchar_t) * (num + 1), "\\Bin32\\glslc.exe");
		string compiledName = name + "-" + extension + ".spv";
		string shaderPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Sources) + name + "." + extension;
		string outputPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + compiledName;

		const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
		string pOut = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + name + "-" + extension + "-compile.log";

		// create a process to use vulkanSDK to compile shader sources to binary (spirv)
		if (System::GetInstance()->RunProcess(glslc, args, 3, pOut.c_str()) != 0)
		{
			SG_LOG_WARN("%s", pOut);
			SG_ASSERT(false);
		}

		return eastl::move(compiledName);
	}

	bool ShaderVk::ReadBinaryFromDisk(const string& name, const string& extension)
	{
		auto* pFS = System::GetInstance()->GetIFileSystem();
		string filepath = "";
		if (extension == "spv")
			filepath = name + "." + extension;
		else
			filepath = name + "-" + extension + ".spv";
		if (pFS->Open(EResourceDirectory::eShader_Binarires, filepath.c_str(), EFileMode::eRead_Binary))
		{
			Size fileSize = pFS->FileSize();
			std::byte* pBinary = (std::byte*)Malloc(fileSize * sizeof(std::byte));
			pFS->Read(pBinary, fileSize);
			Size binarySize = fileSize * sizeof(std::byte);
			pFS->Close();

			string actualExtension = extension;
			if (extension == "spv")
			{
				Size slashPos = name.find_last_of('-');
				actualExtension = name.substr(slashPos + 1, name.size() - slashPos);
			}

			if (actualExtension == "vert")
				mShaderStages[EShaderStages::eVert] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "frag")
				mShaderStages[EShaderStages::eFrag] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "comp")
				mShaderStages[EShaderStages::eComp] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "gemo")
				mShaderStages[EShaderStages::eGeom] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "tese")
				mShaderStages[EShaderStages::eTese] = { pBinary, binarySize, VK_NULL_HANDLE };
			else if (actualExtension == "tesc")
				mShaderStages[EShaderStages::eTesc] = { pBinary, binarySize, VK_NULL_HANDLE };

			return true;
		}
		else
			return false;
	}

	void ShaderVk::CreateVulkanShaderModule(ShaderData* pShaderData)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = pShaderData->binarySize;
		createInfo.pCode = reinterpret_cast<UInt32*>(pShaderData->pBinary);

		VkShaderModule shaderModule = {};
		if (vkCreateShaderModule((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create shader module from spirv!");
			SG_ASSERT(false);
		}
		pShaderData->pShader = shaderModule;
	}

}