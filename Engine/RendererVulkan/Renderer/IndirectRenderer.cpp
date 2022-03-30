#include "StdAfx.h"
#include "IndirectRenderer.h"

#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanCommandBuffer* IndirectRenderer::mpCmdBuf = nullptr;

	eastl::fixed_map<EMeshPass, vector<DrawIndexedIndirectCommand>, (UInt32)EMeshPass::NUM_MESH_PASS> IndirectRenderer::mDrawCallMap;
	eastl::fixed_map<EMeshPass, IndirectDrawCall, (UInt32)EMeshPass::NUM_MESH_PASS> IndirectRenderer::mIndirectDrawCallMap;
	UInt32 IndirectRenderer::mPackedVBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedIBCurrOffset = 0;

	bool IndirectRenderer::mbBeginDraw = false;
	bool IndirectRenderer::mbDrawCallReady = false;

	void IndirectRenderer::CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder)
	{
		pRenderDataBuilder->TraverseRenderData([&](UInt32 meshId, const RenderMeshBuildData& buildData)
			{
				if (buildData.instanceCount > 1) // move it to the Forward Instance Mesh Pass
				{
					BufferCreateDesc bufferCreateDesc = {};
					bufferCreateDesc.name = (string("instance_vb_") + eastl::to_string(meshId)).c_str();
					bufferCreateDesc.pInitData = buildData.perInstanceData.data();
					bufferCreateDesc.bufferSize = sizeof(float) * 4 * buildData.instanceCount;
					bufferCreateDesc.type = EBufferType::efVertex;

					VK_RESOURCE()->CreateBuffer(bufferCreateDesc, false);
					SG_LOG_DEBUG("Have instance!");
				}

				auto* pMeshData = MeshDataArchive::GetInstance()->GetData(meshId);
				const UInt64 vbSize = pMeshData->vertices.size() * sizeof(float);
				const UInt64 ibSize = pMeshData->indices.size() * sizeof(UInt32);

				if (mPackedVBCurrOffset + vbSize > SG_MAX_PACKED_VERTEX_BUFFER_SIZE)
				{
					SG_LOG_ERROR("Vertex buffer exceed boundary!");
					SG_ASSERT(false);
				}
				if (mPackedIBCurrOffset + ibSize > SG_MAX_PACKED_INDEX_BUFFER_SIZE)
				{
					SG_LOG_ERROR("Index buffer exceed boundary!");
					SG_ASSERT(false);
				}

				// create one big vertex buffer
				BufferCreateDesc vbCI = {};
				vbCI.name = "packed_vertex_buffer_0";
				vbCI.bufferSize = SG_MAX_PACKED_VERTEX_BUFFER_SIZE;
				vbCI.type = EBufferType::efVertex;
				vbCI.pInitData = pMeshData->vertices.data();
				vbCI.dataSize = static_cast<UInt32>(vbSize);
				vbCI.dataOffset = static_cast<UInt32>(mPackedVBCurrOffset);
				vbCI.bSubBufer = true;
				VK_RESOURCE()->CreateBuffer(vbCI, true);

				// create one big index buffer
				BufferCreateDesc ibCI = {};
				ibCI.name = "packed_index_buffer_0";
				ibCI.bufferSize = SG_MAX_PACKED_INDEX_BUFFER_SIZE;
				ibCI.type = EBufferType::efIndex;
				ibCI.pInitData = pMeshData->indices.data();
				ibCI.dataSize = static_cast<UInt32>(ibSize);
				ibCI.dataOffset = static_cast<UInt32>(mPackedIBCurrOffset);
				ibCI.bSubBufer = true;
				VK_RESOURCE()->CreateBuffer(ibCI, true);
				VK_RESOURCE()->FlushBuffers();

				DrawIndexedIndirectCommand indirect;
				indirect.firstIndex = mPackedIBCurrOffset / sizeof(UInt32);
				indirect.firstInstance = buildData.instanceCount == 1 ? buildData.objectId : 0;
				indirect.indexCount = static_cast<UInt32>(ibSize / sizeof(UInt32));
				indirect.instanceCount = buildData.instanceCount;
				indirect.vertexOffset = mPackedVBCurrOffset / (sizeof(float) * 6);

				if (buildData.instanceCount == 1)
				{
					mDrawCallMap[EMeshPass::eForward].emplace_back(eastl::move(indirect));
				}
				else
				{
					mDrawCallMap[EMeshPass::eForwardInstanced].emplace_back(eastl::move(indirect));
				}

				mPackedVBCurrOffset += static_cast<UInt32>(vbSize);
				mPackedIBCurrOffset += static_cast<UInt32>(ibSize);
			});

		BufferCreateDesc indirectCI = {};
		indirectCI.name = "indirectBuffer";
		indirectCI.bufferSize = sizeof(DrawIndexedIndirectCommand) * SG_MAX_DRAW_CALL;
		indirectCI.type = EBufferType::efIndirect;
		VK_RESOURCE()->CreateBuffer(indirectCI);

		UInt32 offset = 0;
		auto* pIndirectBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer");

		{
			auto& indirectBuffers = mDrawCallMap[EMeshPass::eForward];
			pIndirectBuffer->UploadData(indirectBuffers.data(), static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand) * indirectBuffers.size()), offset);

			IndirectDrawCall indirectDrawCall;
			indirectDrawCall.drawMesh.pVertexBuffer = VK_RESOURCE()->GetBuffer("packed_vertex_buffer_0");
			indirectDrawCall.drawMesh.pIndexBuffer = VK_RESOURCE()->GetBuffer("packed_index_buffer_0");
			indirectDrawCall.count = static_cast<UInt32>(indirectBuffers.size());
			indirectDrawCall.first = offset / sizeof(DrawIndexedIndirectCommand);

			offset += static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand) * indirectBuffers.size());
			mIndirectDrawCallMap[EMeshPass::eForward] = eastl::move(indirectDrawCall);
		}

		{

			auto& indirectInstanceBuffers = mDrawCallMap[EMeshPass::eForwardInstanced];
			pIndirectBuffer->UploadData(indirectInstanceBuffers.data(), static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand) * indirectInstanceBuffers.size()), offset);

			IndirectDrawCall indirectDrawCall;
			indirectDrawCall.drawMesh.pVertexBuffer = VK_RESOURCE()->GetBuffer("packed_vertex_buffer_0");
			indirectDrawCall.drawMesh.pIndexBuffer = VK_RESOURCE()->GetBuffer("packed_index_buffer_0");
			indirectDrawCall.drawMesh.pInstanceBuffer = VK_RESOURCE()->GetBuffer("instance_vb_1");
			indirectDrawCall.count = static_cast<UInt32>(indirectInstanceBuffers.size());
			indirectDrawCall.first = offset / sizeof(DrawIndexedIndirectCommand);

			offset += static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand) * indirectInstanceBuffers.size());
			mIndirectDrawCallMap[EMeshPass::eForwardInstanced] = eastl::move(indirectDrawCall);
		}

		mbDrawCallReady = true;
	}

	void IndirectRenderer::Begin(VulkanCommandBuffer* pCmdBuf)
	{
		if (mbBeginDraw)
		{
			SG_LOG_ERROR("Did you forget to call End() after drawing?");
			return;
		}
		if (!mbDrawCallReady)
		{
			SG_LOG_WARN("Please call CollectRenderData() before your drawing!");
			return;
		}

		mbBeginDraw = true;
		mpCmdBuf = pCmdBuf;
	}

	void IndirectRenderer::End()
	{
		if (!mbBeginDraw)
		{
			SG_LOG_ERROR("Did you forget to call Begin() before drawing?");
			return;
		}

		mbBeginDraw = false;
		mpCmdBuf = nullptr;
	}

	void IndirectRenderer::Draw(EMeshPass meshPass)
	{
		// TODO: think of a more generic way to draw.
		if (meshPass == EMeshPass::eForward)
		{
			auto& dc = mIndirectDrawCallMap[meshPass];
			BindMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(VK_RESOURCE()->GetBuffer("indirectBuffer"), dc.first * sizeof(DrawIndexedIndirectCommand), dc.count, sizeof(DrawIndexedIndirectCommand));
		}
		else if (meshPass == EMeshPass::eForwardInstanced)
		{
			auto& dc = mIndirectDrawCallMap[meshPass];
			BindInstanceMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(VK_RESOURCE()->GetBuffer("indirectBuffer"), dc.first * sizeof(DrawIndexedIndirectCommand), dc.count, sizeof(DrawIndexedIndirectCommand));
		}
	}

	void IndirectRenderer::BindMesh(const DrawMesh& drawMesh)
	{
		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void IndirectRenderer::BindInstanceMesh(const DrawMesh& drawMesh)
	{
		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		mpCmdBuf->BindVertexBuffer(1, 1, *drawMesh.pInstanceBuffer, &drawMesh.instanceOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void IndirectRenderer::BindMaterial(const DrawMaterial& drawMaterial)
	{

	}

}