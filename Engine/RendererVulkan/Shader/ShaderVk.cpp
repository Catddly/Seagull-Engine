#include "StdAfx.h"
#include "ShaderVk.h"

#include "Core/System/System.h"
#include "Common/System/IFileSystem.h"

#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "Common/Memory/IMemory.h"

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
			if (!ReadBinaryFromDisk(string(filepath).c_str())) // no spirv file exist, try to compile the file from ShaderSrc
			{
				Size slashPos = filepath.find_last_of('-');
				string compiledPath = string(filepath).substr(0, dotPos);  // convert to string to ensure the null-terminated string
				compiledPath[slashPos] = '.';
				string name = compiledPath.substr(0, slashPos);
				string extension = compiledPath.substr(slashPos + 1, compiledPath.size() - slashPos);
				CompileFromVulkanSDK(name, extension);
				if (!ReadBinaryFromDisk(string(filepath).c_str()))
				{
					SG_LOG_ERROR("Failed to compile shader!");
					SG_ASSERT(false);
				}
			}
		}
		else if (extension == "vert" || extension == "frag" || extension == "comp" || extension == "geom") // compiled the source shader to spirv
		{
			string filepath = string(name) + "-" + string(extension) + ".spv";
			if (!ReadBinaryFromDisk(string(filepath).c_str())) // no spirv file exist, try to compile the file from ShaderSrc
			{
				CompileFromVulkanSDK(string(name), string(extension));
				if (!ReadBinaryFromDisk(filepath.c_str()))
				{
					SG_LOG_ERROR("Failed to compile shader!");
					SG_ASSERT(false);
				}
			}
		}
		else
		{
			SG_LOG_WARN("Unknown file type of shader!");
			SG_ASSERT(false);
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = mBinarySize;
		createInfo.pCode = reinterpret_cast<UInt32*>(mBinary);

		if (vkCreateShaderModule((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
		{
			SG_LOG_ERROR("Failed to create shader module from spirv!");
			SG_ASSERT(false);
		}
	}

	ShaderVk::~ShaderVk()
	{
		if (mBinary)
			Free(mBinary);
		vkDestroyShaderModule((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mShaderModule, nullptr);
	}

	string ShaderVk::CompileFromVulkanSDK(const string& name, const string& extension) const
	{
		// prepare for compile
		string glslangValidator = ::getenv("VULKAN_SDK");
		glslangValidator += "/Bin32/glslc.exe";
		string compiledName = name + "-" + extension + ".spv";
		string shaderPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Sources) + name + "." + extension;
		string outputPath = System::GetInstance()->GetResourceDirectory(EResourceDirectory::eShader_Binarires) + compiledName;

		const char* args[3] = { shaderPath.c_str(), "-o", outputPath.c_str() };
		const char* pOut = "";

		// create a process to use vulkanSDK to compile shader sources to binary (spirv)
		System::GetInstance()->RunProcess(glslangValidator.c_str(), args, 3, pOut);

		return eastl::move(compiledName);
	}

	bool ShaderVk::ReadBinaryFromDisk(const char* filepath)
	{
		auto* pFS = System::GetInstance()->GetIFileSystem();
		if (pFS->Open(EResourceDirectory::eShader_Binarires, filepath, EFileMode::eRead_Binary))
		{
			Size fileSize = pFS->FileSize();
			mBinary = (std::byte*)Malloc(fileSize * sizeof(std::byte));
			pFS->Read(mBinary, fileSize);
			mBinarySize = fileSize * sizeof(std::byte);
			pFS->Close();
			return true;
		}
		else
			return false;
	}

}