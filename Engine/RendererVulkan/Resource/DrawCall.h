#pragma once

#include "RendererVulkan/Backend/VulkanBuffer.h"

namespace SG
{

	struct DrawCall
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
	};

}