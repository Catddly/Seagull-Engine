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

	struct DrawMesh
	{
		UInt64 vBSize = 0;
		UInt64 vBOffset = 0;
		VulkanBuffer* pVertexBuffer = nullptr;

		UInt64 iBSize = 0;
		UInt64 iBOffset = 0;
		VulkanBuffer* pIndexBuffer = nullptr;

		UInt64 instanceOffset = 0;
		VulkanBuffer* pInstanceBuffer = nullptr;
	};

	struct DrawMaterial
	{

	};

	struct DrawCall
	{
		DrawMesh drawMesh;
		DrawMaterial drawMaterial;

		UInt32 objectId;
		UInt32 instanceCount = 1;
	};

	struct IndirectDrawCall
	{
		DrawMesh drawMesh;
		DrawMaterial drawMaterial;

		UInt32 first = 0;
		UInt32 count = 0;
	};

}