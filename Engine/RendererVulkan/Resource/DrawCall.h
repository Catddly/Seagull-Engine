#pragma once

#include "RendererVulkan/Backend/VulkanBuffer.h"

namespace SG
{

	struct DrawIndirectCommand
	{
		UInt32 vertexCount;
		UInt32 instanceCount;
		UInt32 firstVertex;
		UInt32 firstInstance;
	};

	struct DrawIndexedIndirectCommand
	{
		UInt32 indexCount;
		UInt32 instanceCount;
		UInt32 firstIndex;
		Int32  vertexOffset;
		UInt32 firstInstance;
	};

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