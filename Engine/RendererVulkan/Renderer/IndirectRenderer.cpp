#include "StdAfx.h"
#include "IndirectRenderer.h"

#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanCommandBuffer* IndirectRenderer::mpCmdBuf = nullptr;

	eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> IndirectRenderer::mDrawCallMap;
	UInt32 IndirectRenderer::mPackedVBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedIBCurrOffset = 0;

	bool IndirectRenderer::mbRendererInit = false;
	bool IndirectRenderer::mbBeginDraw = false;
	bool IndirectRenderer::mbDrawCallReady = false;

	void IndirectRenderer::OnInit()
	{
		BufferCreateDesc indirectCI = {};
		indirectCI.name = "indirectBuffer";
		indirectCI.bufferSize = sizeof(DrawIndexedIndirectCommand) * SG_MAX_DRAW_CALL;
		indirectCI.type = EBufferType::efIndirect | EBufferType::efStorage;
		if (VK_RESOURCE()->CreateBuffer(indirectCI))
			mbRendererInit = true;
	}

	void IndirectRenderer::OnShutdown()
	{
		VK_RESOURCE()->DeleteBuffer("indirectBuffer");
		mbRendererInit = false;
	}

	void IndirectRenderer::CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder)
	{
		if (!mbRendererInit)
		{
			SG_LOG_ERROR("Renderer not initialized");
			return;
		}

		vector<DrawIndexedIndirectCommand> indirectCommands;
		pRenderDataBuilder->TraverseRenderData([&](UInt32 meshId, const RenderMeshBuildData& buildData)
			{
				if (buildData.instanceCount > 1) // move it to the Forward Instance Mesh Pass
				{
					BufferCreateDesc bufferCreateDesc = {};
					bufferCreateDesc.name = (string("instance_vb_") + eastl::to_string(meshId)).c_str();
					bufferCreateDesc.pInitData = buildData.perInstanceData.data();
					bufferCreateDesc.bufferSize = sizeof(PerInstanceData) * buildData.instanceCount;
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

				IndirectDrawCall indirectDc;
				indirectDc.drawMesh.pVertexBuffer = VK_RESOURCE()->GetBuffer("packed_vertex_buffer_0");
				indirectDc.drawMesh.vBSize = vbSize;
				indirectDc.drawMesh.vBOffset = mPackedVBCurrOffset;
				indirectDc.drawMesh.pIndexBuffer = VK_RESOURCE()->GetBuffer("packed_index_buffer_0");;
				indirectDc.drawMesh.iBSize = ibSize;
				indirectDc.drawMesh.iBOffset = mPackedIBCurrOffset;
				indirectDc.drawMesh.pInstanceBuffer = nullptr;
				indirectDc.drawMesh.instanceOffset = 0;

				indirectDc.count = 1;
				indirectDc.first = static_cast<UInt32>(indirectCommands.size());

				if (buildData.instanceCount == 1)
				{
					mDrawCallMap[EMeshPass::eForward].emplace_back(eastl::move(indirectDc));
				}
				else
				{
					indirectDc.drawMesh.pInstanceBuffer = VK_RESOURCE()->GetBuffer((string("instance_vb_") + eastl::to_string(meshId)));
					mDrawCallMap[EMeshPass::eForwardInstanced].emplace_back(eastl::move(indirectDc));
				}

				DrawIndexedIndirectCommand indirect;
				//indirect.firstIndex = mPackedIBCurrOffset / sizeof(UInt32);
				indirect.firstIndex = 0;
				indirect.firstInstance = buildData.instanceCount == 1 ? buildData.objectId : 0;
				indirect.indexCount = static_cast<UInt32>(ibSize / sizeof(UInt32));
				indirect.instanceCount = buildData.instanceCount;
				indirect.vertexOffset = 0;
				//indirect.vertexOffset = mPackedVBCurrOffset / (sizeof(float) * 6);

				indirectCommands.emplace_back(eastl::move(indirect));

				mPackedVBCurrOffset += static_cast<UInt32>(vbSize);
				mPackedIBCurrOffset += static_cast<UInt32>(ibSize);
			});

		UInt32 offset = 0;
		auto* pIndirectBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer");
		pIndirectBuffer->UploadData(indirectCommands.data(), static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand) * indirectCommands.size()), offset);

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
		for (auto& dc : mDrawCallMap[meshPass])
		{
			BindMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(VK_RESOURCE()->GetBuffer("indirectBuffer"), dc.first * sizeof(DrawIndexedIndirectCommand), dc.count, sizeof(DrawIndexedIndirectCommand));
		}
	}

	void IndirectRenderer::BindMesh(const DrawMesh& drawMesh)
	{
		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		if (drawMesh.pInstanceBuffer)
			mpCmdBuf->BindVertexBuffer(1, 1, *drawMesh.pInstanceBuffer, &drawMesh.instanceOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void IndirectRenderer::BindMaterial(const DrawMaterial& drawMaterial)
	{

	}

}