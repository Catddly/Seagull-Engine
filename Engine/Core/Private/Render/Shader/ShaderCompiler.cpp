#include "StdAfx.h"
#include "Render/Shader/ShaderComiler.h"

#include "System/System.h"
#include "System/FileSystem.h"
#include "System/Logger.h"
#include "Memory/Memory.h"
#include "Render/Shader/ShaderLibrary.h"

#include "spirv-cross/spirv_cross.hpp"

#include "eastl/set.h"

namespace SG
{

	static EShaderDataType _SPIRVTypeToShaderDataType(const spirv_cross::SPIRType& type)
	{
		if (type.basetype == spirv_cross::SPIRType::Float)
		{
			if (type.columns == 1)
			{
				if (type.vecsize == 1)
					return EShaderDataType::eFloat;
				else if (type.vecsize == 2)
					return EShaderDataType::eFloat2;
				else if (type.vecsize == 3)
					return EShaderDataType::eFloat3;
				else if (type.vecsize == 4)
					return EShaderDataType::eUnorm4; // TODO: what is the different of eFloat4 and eUnorm4?
			}
			else if (type.columns == 3 && type.vecsize == 3)
				return EShaderDataType::eMat3;
			else if (type.columns == 4 && type.vecsize == 4)
				return EShaderDataType::eMat4;
		}
		else if (type.basetype == spirv_cross::SPIRType::Int)
		{
			if (type.vecsize == 1)
				return EShaderDataType::eInt;
			else if (type.vecsize == 2)
				return EShaderDataType::eInt2;
			else if (type.vecsize == 3)
				return EShaderDataType::eInt3;
			else if (type.vecsize == 4)
				return EShaderDataType::eInt4;
		}
		else if (type.basetype == spirv_cross::SPIRType::Boolean)
			return EShaderDataType::eBool;

		SG_LOG_ERROR("Unsupported shader data type yet!");
		SG_ASSERT(false);
		return EShaderDataType::eUndefined;
	}

	bool ShaderCompiler::LoadSPIRVShader(const string& binShaderName, Shader* pShader)
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

			ReadInShaderData(actualName, i, pShader, shaderBits);
			pShader->mShaderStages[EShaderStage(1 << i)].name = binShaderName;
		}

		//if ((shaderBits & (1 << 0)) == 0 || (shaderBits & (1 << 4)) == 0) // if vert or frag stage is missing
		//{
		//	pShader->mShaderStages.clear();

		//	SG_LOG_WARN("Necessary shader stages(vert or frag) is/are missing!");
		//	return false;
		//}

		if (shaderBits == 0)
		{
			SG_LOG_ERROR("No SPIRV shader is found! (%s)", binShaderName.c_str());
			return false;
		}
	
		return ReflectSPIRV(pShader);
	}

	bool ShaderCompiler::LoadSPIRVShader(const string& vertShaderName, const string& fragShaderName, Shader* pShader)
	{
		UInt8 shaderBits = 0;
		if (!FileSystem::Exist(EResourceDirectory::eShader_Binarires, ""))
		{
			SG_LOG_WARN("You must put all the SPIRV binaries in the ShaderBin folder!");
			return false;
		}

		string vertName = vertShaderName + "-vert.spv";
		string fragName = fragShaderName + "-frag.spv";

		ReadInShaderData(vertName, 0, pShader, shaderBits);
		ReadInShaderData(fragName, 4, pShader, shaderBits);

		if ((shaderBits & (1 << 0)) == 0 || (shaderBits & (1 << 4)) == 0) // if vert or frag stage is missing
		{
			pShader->mShaderStages.clear();

			SG_LOG_WARN("Necessary shader stages(vert or frag) is/are missing!");
			return false;
		}

		if (shaderBits == 0)
		{
			SG_LOG_ERROR("No SPIRV shader is found! (vert: %s, frag: %s)", vertShaderName.c_str(), fragShaderName.c_str());
			return false;
		}

		pShader->mShaderStages[EShaderStage::efVert].name = vertShaderName;
		pShader->mShaderStages[EShaderStage::efFrag].name = fragShaderName;
		return ReflectSPIRV(pShader);
	}

	bool ShaderCompiler::CompileGLSLShader(const string& binShaderName, Shader* pShader)
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
			if (FileSystem::Exist(EResourceDirectory::eShader_Sources, actualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
			{
				bool bForceToRecompile = false;
				// Get the time stamp of this shader to check if this shader should force to recompile.
				TimePoint prevTp = ShaderLibrary::GetInstance()->GetShaderTimeStamp(actualName);
				TimePoint currTp = FileSystem::GetFileLastWriteTime(EResourceDirectory::eShader_Sources, actualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET);
				if (!prevTp.IsValid() || currTp > prevTp) // invalid means we can't find shader_env.ini, so you should recompile the shader anyway.
					bForceToRecompile = true;

				string compiledName = actualName;
				compiledName[actualName.find('.')] = '-';
				compiledName += ".spv";

				// Skip this to force recompile shader.
				if (!bForceToRecompile && FileSystem::Exist(EResourceDirectory::eShader_Binarires, compiledName.c_str())) // already compiled this shader once, skip it.
				{
					shaderBits |= (1 << i); // mark as exist.
					continue;
				}

				string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + binShaderName + "-" +
					extension.substr(1, extension.size() - 1) + "-compile.log";

				if (CompileShaderVkSDK(actualName, compiledName, commandLine, pOut))
					shaderBits |= (1 << i); // record what shader stage we had compiled
				else
					SG_LOG_ERROR("Failed to compile shader: %s", actualName.c_str());
			}
		}

		if (shaderBits == 0)
			return false;

		return LoadSPIRVShader(binShaderName, pShader);
	}

	bool ShaderCompiler::CompileGLSLShader(const string& vertShaderName, const string& fragShaderName, Shader* pShader)
	{
		char* glslc = "";
		Size num = 1;
		_dupenv_s(&glslc, &num, "VULKAN_SDK");
		string glslcPath = glslc;
		glslcPath += "\\Bin32\\glslc.exe ";

		UInt8 shaderBits = 0;
		FileSystem::ExistOrCreate(EResourceDirectory::eShader_Binarires, ""); // create ShaderBin folder if it doesn't exist

		string vertActualName = vertShaderName + ".vert";
		if (FileSystem::Exist(EResourceDirectory::eShader_Sources, vertActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			bool bForceToRecompile = false;
			// Get the time stamp of this shader to check if this shader should force to recompile.
			TimePoint prevTp = ShaderLibrary::GetInstance()->GetShaderTimeStamp(vertActualName);
			TimePoint currTp = FileSystem::GetFileLastWriteTime(EResourceDirectory::eShader_Sources, vertActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET);
			if (!prevTp.IsValid() || currTp > prevTp) // invalid means we can't find shader_env.ini, so you should recompile the shader anyway.
				bForceToRecompile = true;

			string vertCompiledName = vertActualName;
			vertCompiledName[vertActualName.find('.')] = '-';
			vertCompiledName += ".spv";

			if (!bForceToRecompile && FileSystem::Exist(EResourceDirectory::eShader_Binarires, vertCompiledName.c_str())) // already compiled this shader once, skip it.
				shaderBits |= (1 << 0); // mark as exist.

			string commandLine = glslcPath;
			string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + vertShaderName + "-vert-compile.log";

			if (CompileShaderVkSDK(vertActualName, vertCompiledName, commandLine, pOut))
				shaderBits |= (1 << 0); // record what shader stage we had compiled
			else
				SG_LOG_ERROR("Failed to compile vertex shader: %s", vertActualName.c_str());
		}

		string fragActualName = fragShaderName + ".frag";
		if (FileSystem::Exist(EResourceDirectory::eShader_Sources, fragActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			bool bForceToRecompile = false;
			// Get the time stamp of this shader to check if this shader should force to recompile.
			TimePoint prevTp = ShaderLibrary::GetInstance()->GetShaderTimeStamp(vertActualName);
			TimePoint currTp = FileSystem::GetFileLastWriteTime(EResourceDirectory::eShader_Sources, fragActualName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET);
			if (currTp > prevTp)
				bForceToRecompile = true;

			string fragCompiledName = fragActualName;
			fragCompiledName[fragActualName.find('.')] = '-';
			fragCompiledName += ".spv";

			if (FileSystem::Exist(EResourceDirectory::eShader_Binarires, fragCompiledName.c_str())) // already compiled this shader once, skip it.
				shaderBits |= (1 << 4); // mark as exist.

			string commandLine = glslcPath;
			string pOut = FileSystem::GetResourceFolderPath(EResourceDirectory::eShader_Binarires) + fragShaderName + "-frag-compile.log";

			if (CompileShaderVkSDK(fragActualName, fragCompiledName, commandLine, pOut))
				shaderBits |= (1 << 4); // record what shader stage we had compiled
			else
				SG_LOG_ERROR("Failed to compile fragment shader: %s", fragActualName.c_str());
		}

		if (shaderBits == 0)
			return false;

		return LoadSPIRVShader(vertShaderName, fragShaderName, pShader);
	}

	void ShaderCompiler::ReadInShaderData(const string& name, UInt32 stage, Shader* pShader, UInt8& checkFlag)
	{
		if (FileSystem::Open(EResourceDirectory::eShader_Binarires, name.c_str(), EFileMode::efRead_Binary))
		{
			const Size sizeInByte = FileSystem::FileSize();
			pShader->mShaderStages[(EShaderStage)(1 << stage)] = {}; // insert a null vector
			auto& shaderBinary = pShader->mShaderStages[(EShaderStage)(1 << stage)];
			shaderBinary.binary.resize(sizeInByte);

			FileSystem::Read(shaderBinary.binary.data(), sizeInByte);

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

	// helper to keep the binding or the location ordered
	template <typename ElementType>
	struct ShaderAttributesLayoutLocationComparer
	{
		bool operator()(const eastl::pair<UInt32, typename ElementType>& lhs,
			const eastl::pair<UInt32, typename ElementType>& rhs)
		{
			return lhs.first < rhs.first;
		}
	};
	template <typename ElementType>
	using OrderSet = eastl::set<eastl::pair<UInt32, typename ElementType>, ShaderAttributesLayoutLocationComparer<typename ElementType>>;

	bool ShaderCompiler::ReflectSPIRV(Shader* pShader)
	{
		for (auto& beg = pShader->mShaderStages.begin(); beg != pShader->mShaderStages.end(); ++beg)
		{
			auto& shaderData = beg->second;
			if (shaderData.binary.empty())
				continue;

			spirv_cross::Compiler compiler(reinterpret_cast<const UInt32*>(shaderData.binary.data()), shaderData.binary.size() / sizeof(UInt32));
			auto& stageData = compiler.get_entry_points_and_stages();

			// we only have one shader stage compile once for now.
			spv::ExecutionModel executionModel = {};
			for (auto& data : stageData)
			{
				pShader->mEntryPoint = data.name.c_str();
				executionModel = data.execution_model;
			}

			spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

			// shader stage input collection
			if (executionModel == spv::ExecutionModelVertex) // for now, we only collect the attributes of vertex stage
			{
				OrderSet<ShaderAttributesLayout::ElementType> orderedInputLayout;
				for (auto& input : shaderResources.stage_inputs) // collect shader stage input info
				{
					const auto& type = compiler.get_type(input.type_id);
					const auto location = compiler.get_decoration(input.id, spv::DecorationLocation);
					orderedInputLayout.emplace(location, ShaderAttributesLayout::ElementType{ _SPIRVTypeToShaderDataType(type), input.name.c_str() });
				}

				for (auto& element : orderedInputLayout)
					shaderData.stageInputLayout.Emplace(eastl::move(element.second));
			}

			// shader push constants collection
			for (auto& pushConstant : shaderResources.push_constant_buffers)
			{
				const auto& type = compiler.get_type(pushConstant.type_id);
				if (type.basetype == spirv_cross::SPIRType::Struct)
				{
					for (UInt32 i = 0; i < type.member_types.size(); ++i)
					{
						const auto memberTypeID = type.member_types[i];
						const char* name = compiler.get_member_name(type.self, i).c_str();
						const auto& memberType = compiler.get_type(memberTypeID);
						shaderData.pushConstantLayout.Emplace(_SPIRVTypeToShaderDataType(memberType), name);
					}
				}
				else
				{
					const char* name = compiler.get_name(pushConstant.id).c_str();
					shaderData.pushConstantLayout.Emplace(_SPIRVTypeToShaderDataType(type), name);
				}
			}

			// shader uniform buffers collection
			for (auto& ubo : shaderResources.uniform_buffers)
			{
				const auto& type = compiler.get_type(ubo.type_id);
				const char* name = compiler.get_name(ubo.id).c_str();
				const auto set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
				const auto binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
				const UInt32 key = set * 10 + binding; // calculate key value for the set and binding

				if (pShader->mUniformBufferLayout.Exist(name)) // may be another stage is using it, too.
				{
					auto& data = pShader->mUniformBufferLayout.Get(name);
					data.stage = data.stage | beg->first;
					continue;
				}

				ShaderAttributesLayout layout = {};
				if (type.basetype == spirv_cross::SPIRType::Struct)
				{
					for (UInt32 i = 0; i < type.member_types.size(); ++i)
					{
						const auto memberTypeID = type.member_types[i];
						const char* memberName = compiler.get_member_name(type.self, i).c_str();
						const auto& memberType = compiler.get_type(memberTypeID);
						layout.Emplace(_SPIRVTypeToShaderDataType(memberType), memberName);
					}
				}
				else
				{
					const char* memberName = compiler.get_name(ubo.id).c_str();
					layout.Emplace(_SPIRVTypeToShaderDataType(type), memberName);
				}

				pShader->mUniformBufferLayout.Emplace(name, { key, layout, beg->first });
			}

			// shader combine sampler image collection
			for (auto& image : shaderResources.sampled_images)
			{
				const char* name = compiler.get_name(image.id).c_str();
				const auto set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
				const auto binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				const UInt32 key = set * 10 + binding; // calculate key value for the set and binding

				shaderData.sampledImageLayout.Emplace(name, key);
			}
		}

		return true;
	}

}