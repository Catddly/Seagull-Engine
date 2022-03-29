#include "StdAfx.h"
#include "RendererVulkan/IndirectDraw/IndirectDrawBatcher.h"

#include "RendererVulkan/Backend/VulkanCommand.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Resource/DrawCall.h"

namespace SG
{

	void IndirectDrawBatcher::CreateIndirectBuffer()
	{
		BufferCreateDesc indirectCI = {};
		indirectCI.name = "indirectBuffer";
		indirectCI.totalSizeInByte = sizeof(DrawIndexedIndirectCommand) * SG_MAX_DRAW_CALL;
		indirectCI.type = EBufferType::efIndirect;
		VK_RESOURCE()->CreateBuffer(indirectCI);

		// pre-init mesh passes
		mIndirectBatchesMap.insert(EMeshPass::eForward);
		mIndirectBatchesMap.insert(EMeshPass::eForwardInstanced);
	}

	void IndirectDrawBatcher::DestroyIndirectBuffer()
	{
		VK_RESOURCE()->DeleteBuffer("indirectBuffer");
	}

	void IndirectDrawBatcher::AddMeshPassDrawCall(EMeshPass meshPass, const DrawCall& drawCall)
	{
		DrawIndexedIndirectCommand indexedIndirectCommand;
		indexedIndirectCommand.instanceCount = drawCall.instanceCount;
		indexedIndirectCommand.indexCount = static_cast<UInt32>(drawCall.iBSize / sizeof(UInt32));
		indexedIndirectCommand.vertexOffset = 0;
		indexedIndirectCommand.firstIndex = 0;
		indexedIndirectCommand.firstInstance = drawCall.objectId;

		mIndirectBatchesMap[meshPass].emplace_back(eastl::move(indexedIndirectCommand));

		mbIndirectDrawReady = false;
	}

	void IndirectDrawBatcher::FinishBuildMeshPass(VulkanBuffer* pIndirectCmdBuffer)
	{
		// calculate offset of each render pass
		vector<VkDrawIndexedIndirectCommand> indirectCommands;
		UInt32 offset = 0;
		for (auto& indirectCmd : mIndirectBatchesMap[EMeshPass::eForward])
		{
			VkDrawIndexedIndirectCommand command;
			command.firstIndex = indirectCmd.firstIndex;
			command.firstInstance = indirectCmd.firstInstance;
			command.indexCount = indirectCmd.indexCount;
			command.instanceCount = indirectCmd.instanceCount;
			command.vertexOffset = indirectCmd.vertexOffset;
			indirectCommands.push_back(eastl::move(command));
		}
		mIndirectDrawCalls[(UInt32)EMeshPass::eForward].indirectDrawCount = static_cast<UInt32>(indirectCommands.size());
		mIndirectDrawCalls[(UInt32)EMeshPass::eForward].stride = sizeof(VkDrawIndexedIndirectCommand);
		mIndirectDrawCalls[(UInt32)EMeshPass::eForward].offset = offset;
		UInt32 size = static_cast<UInt32>(sizeof(VkDrawIndexedIndirectCommand) * indirectCommands.size());
		pIndirectCmdBuffer->UploadData(indirectCommands.data(), size, offset);
		offset += size;

		indirectCommands.clear();
		for (auto& indirectCmd : mIndirectBatchesMap[EMeshPass::eForwardInstanced])
		{
			VkDrawIndexedIndirectCommand command;
			command.firstIndex = indirectCmd.firstIndex;
			command.firstInstance = 0;
			command.indexCount = indirectCmd.indexCount;
			command.instanceCount = indirectCmd.instanceCount;
			command.vertexOffset = indirectCmd.vertexOffset;
			indirectCommands.push_back(eastl::move(command));
		}
		mIndirectDrawCalls[(UInt32)EMeshPass::eForwardInstanced].indirectDrawCount = static_cast<UInt32>(indirectCommands.size());
		mIndirectDrawCalls[(UInt32)EMeshPass::eForwardInstanced].stride = sizeof(VkDrawIndexedIndirectCommand);
		mIndirectDrawCalls[(UInt32)EMeshPass::eForwardInstanced].offset = offset;
		size = static_cast<UInt32>(sizeof(VkDrawIndexedIndirectCommand) * indirectCommands.size());
		pIndirectCmdBuffer->UploadData(indirectCommands.data(), size, offset);
		offset += size;

		mbIndirectDrawReady = true;
	}

	void IndirectDrawBatcher::Draw(EMeshPass meshPass, VulkanCommandBuffer& buf)
	{
		if (!mbIndirectDrawReady)
		{
			SG_LOG_ERROR("Please call FinishBuildMeshPass() to build the indirect draw commands!");
			return;
		}

		if (meshPass == EMeshPass::eForward)
		{
			VK_RESOURCE()->TraverseStaticMeshDrawCall([&](const DrawCall& dc)
				{
					buf.BindVertexBuffer(0, 1, *dc.pVertexBuffer, &dc.vBOffset);
					buf.BindIndexBuffer(*dc.pIndexBuffer, dc.iBOffset);

					auto& indirectDrawCall = mIndirectDrawCalls[(UInt32)meshPass];
					buf.DrawIndexedIndirect(VK_RESOURCE()->GetBuffer("indirectBuffer"), indirectDrawCall.offset, indirectDrawCall.indirectDrawCount, indirectDrawCall.stride);
				});
		}
		else if (meshPass == EMeshPass::eForwardInstanced)
		{
			VK_RESOURCE()->TraverseStaticMeshInstancedDrawCall([&](const DrawCall& dc)
				{
					buf.BindVertexBuffer(0, 1, *dc.pVertexBuffer, &dc.vBOffset);
					UInt64 offset = 0;
					buf.BindVertexBuffer(1, 1, *dc.pInstanceBuffer, &offset);
					buf.BindIndexBuffer(*dc.pIndexBuffer, dc.iBOffset);

					auto& indirectDrawCallIns = mIndirectDrawCalls[(UInt32)meshPass];
					buf.DrawIndexedIndirect(VK_RESOURCE()->GetBuffer("indirectBuffer"), indirectDrawCallIns.offset, indirectDrawCallIns.indirectDrawCount, indirectDrawCallIns.stride);
				});
		}
	}

}