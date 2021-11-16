#pragma once

#include "Defs/Defs.h"
#include "RendererVulkan/Config.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"

namespace SG
{

	enum class ERGResourceType
	{
		ePermenant = 0,
		eNormal,
		eTransient,
	};

	//! Resource from the VulkanResourceRegistry.
	//! Use for validation and as a handle to the resource.
	class RenderGraphResource final
	{
	public:
		SG_RENDERER_VK_API explicit RenderGraphResource(const char* name) : mName(name) {}
		SG_RENDERER_VK_API ~RenderGraphResource() = default;

		template <typename T>
		SG_INLINE T* As();
	private:
		const char* mName;
	};

	template <typename T>
	SG_INLINE T* RenderGraphResource::As()
	{
		SG_COMPILE_ASSERT(false, "RenderGraphResource must be one of the render resources!");
	}

	template <>
	SG_INLINE VulkanBuffer* RenderGraphResource::As<VulkanBuffer>()
	{
		auto* pBuffer = VulkanResourceRegistry::GetInstance()->GetBuffer(mName);
		if (!pBuffer)
		{
			SG_LOG_ERROR("No buffer named: %s", mName);
			return nullptr;
		}
		return pBuffer;
	}

}