#pragma once

#include "Core/Config.h"
#include "Base/BasicTypes.h"

#include "System/ILogger.h"

#include <cstddef>
#include <eastl/map.h>
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

	enum class EShaderStages : UInt32
	{
		efVert = 1 << 0,
		efTesc = 1 << 1,
		efTese = 1 << 2,
		efGeom = 1 << 3,
		efFrag = 1 << 4,
		efComp = 1 << 5,
		NUM_STAGES = 6,
	};
	SG_ENUM_CLASS_FLAG(UInt32, EShaderStages);

	enum class EShaderDataType
	{
		eUndefined = 0,
		eFloat, eFloat2, eFloat3, eFloat4,
		eMat3, eMat4,
		eInt, eInt2, eInt3, eInt4,
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

	class BufferLayout
	{
	public:
		typedef BufferLayoutElement                       element_t;
		typedef eastl::vector<element_t>::iterator        iterator_t;
		typedef eastl::vector<element_t>::const_iterator  const_iterator_t;

		BufferLayout() : mTotalSize(0) {}
		BufferLayout(std::initializer_list<element_t> elements)
			:mLayouts(elements)
		{
			CalculateLayoutOffsets();
		}

		void   Append(const element_t& e);
		UInt32 GetTotalSize() const { return mTotalSize; }
		Size   GetSize()      const { return mLayouts.size(); }

		iterator_t begin()              { return mLayouts.begin(); }
		iterator_t end()                { return mLayouts.end(); }
		const_iterator_t begin()  const { return mLayouts.begin(); }
		const_iterator_t end()    const { return mLayouts.end(); }
		const_iterator_t cbegin() const { return mLayouts.cbegin(); }
		const_iterator_t cend()   const { return mLayouts.cend(); }
	private:
		SG_CORE_API void CalculateLayoutOffsets();
	private:
		vector<element_t> mLayouts;
		UInt32            mTotalSize;
	};

	//! Data of shader.
	struct ShaderData
	{
		std::byte*    pBinary = nullptr;
		Size          binarySize;
	};

	typedef eastl::map<EShaderStages, ShaderData> Shader;

}