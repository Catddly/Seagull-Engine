#include "StdAfx.h"
#include "RendererVulkan/Renderer/Renderer.h"

#include "System/Logger.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanCommandBuffer* Renderer::mpCmdBuf = nullptr;

	eastl::fixed_map<EMeshPass, vector<DrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> Renderer::mDrawCallMap;
	UInt64 Renderer::mPackedVBCurrOffset = 0;
	UInt64 Renderer::mPackedIBCurrOffset = 0;

	bool Renderer::mbBeginDraw = false;
	bool Renderer::mbDrawCallReady = false;

	void Renderer::CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder)
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
					bufferCreateDesc.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;

					VK_RESOURCE()->CreateBuffer(bufferCreateDesc);
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
				vbCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
				vbCI.bSubBufer = true;
				vbCI.subBufferSize = static_cast<UInt32>(vbSize);
				vbCI.subBufferOffset = static_cast<UInt32>(mPackedVBCurrOffset);
				VK_RESOURCE()->CreateBuffer(vbCI);

				// create one big index buffer
				BufferCreateDesc ibCI = {};
				ibCI.name = "packed_index_buffer_0";
				ibCI.bufferSize = SG_MAX_PACKED_INDEX_BUFFER_SIZE;
				ibCI.type = EBufferType::efIndex;
				ibCI.pInitData = pMeshData->indices.data();
				ibCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
				ibCI.bSubBufer = true;
				ibCI.subBufferSize = static_cast<UInt32>(ibSize);
				ibCI.subBufferOffset = static_cast<UInt32>(mPackedIBCurrOffset);
				VK_RESOURCE()->CreateBuffer(ibCI);
				VK_RESOURCE()->FlushBuffers();

				DrawCall dc = {};
				dc.drawMesh.pVertexBuffer = VK_RESOURCE()->GetBuffer("packed_vertex_buffer_0");
				dc.drawMesh.vBSize = vbSize;
				dc.drawMesh.vBOffset = mPackedVBCurrOffset;
				dc.drawMesh.pIndexBuffer = VK_RESOURCE()->GetBuffer("packed_index_buffer_0");;
				dc.drawMesh.iBSize = ibSize;
				dc.drawMesh.iBOffset = mPackedIBCurrOffset;
				dc.drawMesh.pInstanceBuffer = nullptr;
				dc.drawMesh.instanceOffset = 0;

				dc.objectId = buildData.objectId;
				dc.instanceCount = buildData.instanceCount;

				if (buildData.instanceCount == 1)
				{
					mDrawCallMap[EMeshPass::eForward].emplace_back(eastl::move(dc));
				}
				else
				{
					dc.drawMesh.pInstanceBuffer = VK_RESOURCE()->GetBuffer((string("instance_vb_") + eastl::to_string(meshId)));
					mDrawCallMap[EMeshPass::eForwardInstanced].emplace_back(eastl::move(dc));
				}

				mPackedVBCurrOffset += vbSize;
				mPackedIBCurrOffset += ibSize;
			});
		mbDrawCallReady = true;
	}

	void Renderer::Begin(VulkanCommandBuffer* pCmdBuf)
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

	void Renderer::End()
	{
		if (!mbBeginDraw)
		{
			SG_LOG_ERROR("Did you forget to call Begin() before drawing?");
			return;
		}

		mbBeginDraw = false;
		mpCmdBuf = nullptr;
	}

	void Renderer::Draw(EMeshPass meshPass)
	{
		// TODO: think of a more generic way to draw.
		if (meshPass == EMeshPass::eForward)
		{
			for (auto& dc : mDrawCallMap[meshPass])
			{
				BindMesh(dc.drawMesh);
				BindMaterial(dc.drawMaterial);

				mpCmdBuf->DrawIndexed(static_cast<UInt32>(dc.drawMesh.iBSize / sizeof(UInt32)), 1, 0, 0, dc.objectId /* corresponding to gl_BaseInstance */);
			}
		}
		else if (meshPass == EMeshPass::eForwardInstanced)
		{
			for (auto& dc : mDrawCallMap[meshPass])
			{
				BindInstanceMesh(dc.drawMesh);
				BindMaterial(dc.drawMaterial);

				mpCmdBuf->DrawIndexed(static_cast<UInt32>(dc.drawMesh.iBSize / sizeof(UInt32)), dc.instanceCount, 0, 0, 0);
			}
		}
	}

	void Renderer::BindMesh(const DrawMesh& drawMesh)
	{
		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void Renderer::BindInstanceMesh(const DrawMesh& drawMesh)
	{
		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		mpCmdBuf->BindVertexBuffer(1, 1, *drawMesh.pInstanceBuffer, &drawMesh.instanceOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void Renderer::BindMaterial(const DrawMaterial& drawMaterial)
	{

	}

}