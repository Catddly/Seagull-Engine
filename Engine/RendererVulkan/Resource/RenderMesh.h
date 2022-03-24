#pragma once

#include "RendererVulkan/Resource/CommonUBO.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"

#include "Stl/string.h"

namespace SG
{

	struct RenderMesh
	{
		UInt64 vBSize = 0;
		UInt64 iBSize = 0;
		UInt64 vBOffset = 0;
		UInt64 iBOffset = 0;
		UInt32 objectId;
		UInt32 instanceCount = 1;

		VulkanBuffer* pVertexBuffer = nullptr;
		VulkanBuffer* pIndexBuffer = nullptr;
		VulkanBuffer* pInstanceBuffer = nullptr;

		PerMeshRenderData renderData;
	};

}