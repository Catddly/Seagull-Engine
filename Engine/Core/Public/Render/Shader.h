#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "System/Logger.h"
#include "Memory/Memory.h"

#include <eastl/map.h>
#include <eastl/fixed_map.h>
#include <eastl/utility.h>
#include "stl/vector.h"
#include "stl/string_view.h"

namespace SG
{

	enum class EShaderLanguage
	{
		eGLSL = 0,
		eHLSL,
		eMetal,
	};

	enum class EShaderStage : UInt32
	{
		efVert = 1 << 0,
		efTesc = 1 << 1,
		efTese = 1 << 2,
		efGeom = 1 << 3,
		efFrag = 1 << 4,
		efComp = 1 << 5,
		NUM_STAGES = 6,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EShaderStage);

	enum class EShaderDataType
	{
		eUndefined = 0,
		eFloat, eFloat2, eFloat3, eFloat4,
		eMat3, eMat4,
		eInt, eInt2, eInt3, eInt4,
		eUnorm4,
		eBool,
	};

	static SG_INLINE UInt32 ShaderDataTypeToSize(EShaderDataType type)
	{
		switch (type)
		{
		case SG::EShaderDataType::eUndefined: return 0; break;
		case SG::EShaderDataType::eFloat:	  return sizeof(float);     break;
		case SG::EShaderDataType::eFloat2:	  return sizeof(float) * 2; break;
		case SG::EShaderDataType::eFloat3:	  return sizeof(float) * 3; break;
		case SG::EShaderDataType::eFloat4:	  return sizeof(float) * 4; break;
		case SG::EShaderDataType::eMat3:	  return sizeof(float) * 3 * 3; break;
		case SG::EShaderDataType::eMat4:	  return sizeof(float) * 4 * 4; break;
		case SG::EShaderDataType::eInt:		  return sizeof(int); break;
		case SG::EShaderDataType::eInt2:	  return sizeof(int) * 2; break;
		case SG::EShaderDataType::eInt3:	  return sizeof(int) * 3; break;
		case SG::EShaderDataType::eInt4:	  return sizeof(int) * 4; break;
		case SG::EShaderDataType::eUnorm4:	  return sizeof(UInt32); break;
		case SG::EShaderDataType::eBool:	  return sizeof(bool); break;
		default: SG_LOG_ERROR("Unknown shader data type!"); break;
		}
		return 0;
	}

	struct BufferLayoutElement
	{
		EShaderDataType type;
		string_view     name;
		UInt32          size;
		UInt32          offset;

		BufferLayoutElement(EShaderDataType t, string_view n)
			:type(t), name(n), size(ShaderDataTypeToSize(t)), offset(0)
		{}
	};

	class ShaderAttributesLayout
	{
	public:
		typedef BufferLayoutElement                        ElementType;
		typedef eastl::vector<ElementType>::iterator       IteratorType;
		typedef eastl::vector<ElementType>::const_iterator ConstIteratorType;

		ShaderAttributesLayout() : mTotalSize(0) {}
		ShaderAttributesLayout(std::initializer_list<ElementType> elements)
			:mLayouts(elements)
		{
			CalculateLayoutOffsets();
		}

		template <typename... Args>
		void Emplace(Args&&... args)
		{
			mLayouts.emplace_back(eastl::forward<Args>(args)...); // perfect forwarding
			mLayouts.back().offset = mTotalSize;
			mTotalSize += mLayouts.back().size;
		}

		UInt32 GetTotalSizeInByte() const { return mTotalSize; }
		Size   GetNumAttributes()   const { return mLayouts.size(); }

		IteratorType begin()             { return mLayouts.begin(); }
		IteratorType end()               { return mLayouts.end(); }
		ConstIteratorType begin()  const { return mLayouts.begin(); }
		ConstIteratorType end()    const { return mLayouts.end(); }
		ConstIteratorType cbegin() const { return mLayouts.cbegin(); }
		ConstIteratorType cend()   const { return mLayouts.cend(); }
	private:
		SG_CORE_API void CalculateLayoutOffsets();
	private:
		vector<ElementType> mLayouts;
		UInt32            mTotalSize;
	};

	typedef UInt32 SetBindingKey;
	SG_INLINE static const UInt32 GetSet(SetBindingKey key) { return key / 10; }
	SG_INLINE static const UInt32 GetBinding(SetBindingKey key) { return key % 10; }

	template <typename ElementType>
	class ShaderSetBindingAttributeLayout
	{
	public:
		typedef typename eastl::map<string, typename ElementType>::iterator       IteratorType;
		typedef typename eastl::map<string, typename ElementType>::const_iterator ConstIteratorType;

		void Emplace(const string& name, const ElementType& element);

		IteratorType      begin()        { return mUBODataMap.begin(); }
		IteratorType      end()          { return mUBODataMap.end(); }
		ConstIteratorType begin()  const { return mUBODataMap.begin(); }
		ConstIteratorType end()    const { return mUBODataMap.end(); }
		ConstIteratorType cbegin() const { return mUBODataMap.cbegin(); }
		ConstIteratorType cend()   const { return mUBODataMap.cend(); }
	private:
		friend class ShaderCompiler;
		eastl::map<string, typename ElementType> mUBODataMap;
	};

	struct UniformBufferLayout
	{
		SetBindingKey          setbinding;
		ShaderAttributesLayout layout;
	};

	template <typename ElementType>
	void ShaderSetBindingAttributeLayout<ElementType>::Emplace(const string& name, const ElementType& element)
	{
		if (mUBODataMap.find(name) != mUBODataMap.end())
		{
			SG_LOG_ERROR("Already have a shader uniform buffer layout called: %s", name.c_str());
			SG_ASSERT(false);
		}
		mUBODataMap[name] = element;
	}

	// dump wrapper of shader map
	class Shader
	{
	public:
		typedef vector<Byte> ShaderBinaryType;
		struct ShaderData
		{
			string                 name;
			ShaderBinaryType       binary;
			ShaderAttributesLayout stageInputLayout;
			ShaderAttributesLayout pushConstantLayout;
			ShaderSetBindingAttributeLayout<UniformBufferLayout> uniformBufferLayout;
			ShaderSetBindingAttributeLayout<SetBindingKey> sampledImageLayout;
		};

		Shader() = default;
		virtual ~Shader() = default;

		SG_INLINE const string&   GetName(EShaderStage stage) { return mShaderStages[stage].name; }
		SG_INLINE const string&   GetEntryPoint() const { return mEntryPoint; }
		SG_INLINE EShaderLanguage GetShaderLanguage() const { return mLanguage; }

		SG_INLINE const ShaderAttributesLayout& GetAttributesLayout(EShaderStage stage)
		{
			if (stage != EShaderStage::efVert)
			{
				SG_LOG_DEBUG("Only collect vertex stage input attributes for now!");
				SG_ASSERT(false);
			}
			return mShaderStages[stage].stageInputLayout;
		}
		SG_INLINE const ShaderAttributesLayout& GetPushConstantLayout(EShaderStage stage) { return mShaderStages[stage].pushConstantLayout; }
		SG_INLINE const ShaderSetBindingAttributeLayout<UniformBufferLayout>& GetUniformBufferLayout(EShaderStage stage) { return mShaderStages[stage].uniformBufferLayout; }
		SG_INLINE const ShaderSetBindingAttributeLayout<SetBindingKey>& GetSampledImageLayout(EShaderStage stage) { return mShaderStages[stage].sampledImageLayout; }

		SG_INLINE const Byte* GetBinary(EShaderStage stage)
		{
			auto& shaderData = mShaderStages[stage];
			return shaderData.binary.data();
		}
		SG_INLINE const Size  GetBinarySize(EShaderStage stage)
		{
			auto& shaderData = mShaderStages[stage];
			return shaderData.binary.size();
		}
	protected:
		SG_INLINE void ReleaseBinary()
		{
			for (auto& data : mShaderStages)
				data.second.binary.resize(0); // clear all the memory of the binary
		}
	protected:
		typedef eastl::fixed_map<EShaderStage, ShaderData, (Size)EShaderStage::NUM_STAGES, false> ShaderStageDataType;
		ShaderStageDataType mShaderStages;
	private:
		friend class ShaderCompiler;
		string mEntryPoint = "main"; // default
		EShaderLanguage mLanguage;
	};

}