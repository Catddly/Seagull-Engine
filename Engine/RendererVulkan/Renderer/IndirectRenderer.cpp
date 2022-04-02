#include "StdAfx.h"
#include "IndirectRenderer.h"

#include "Scene/Mesh/MeshDataArchive.h"
#include "Render/Shader/ShaderComiler.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanContext* IndirectRenderer::mpContext = nullptr;
	VulkanCommandBuffer* IndirectRenderer::mpCmdBuf = nullptr;

	eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> IndirectRenderer::mDrawCallMap;
	eastl::unordered_map<UInt32, UInt32> IndirectRenderer::mDrawCallIndexMap; // meshId -> drawCallIndex
	UInt32 IndirectRenderer::mPackedVBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedVIBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedIBCurrOffset = 0;
	UInt32 IndirectRenderer::mCurrDrawCallIndex = 0;

	VulkanCommandBuffer IndirectRenderer::mComputeCmd;
	RefPtr<VulkanPipelineSignature> IndirectRenderer::mpGPUCullingPipelineSignature = nullptr;
	VulkanPipeline* IndirectRenderer::mpGPUCullingPipeline = nullptr;
	RefPtr<VulkanShader> IndirectRenderer::mpGPUCullingShader = nullptr;

	bool IndirectRenderer::mbRendererInit = false;
	bool IndirectRenderer::mbBeginDraw = false;
	bool IndirectRenderer::mbDrawCallReady = false;

	void IndirectRenderer::OnInit(VulkanContext& context)
	{
		mpContext = &context;

		BufferCreateDesc indirectCI = {};
		indirectCI.name = "indirectBuffer";
		indirectCI.bufferSize = sizeof(DrawIndexedIndirectCommand) * SG_MAX_DRAW_CALL;
		indirectCI.type = EBufferType::efIndirect | EBufferType::efStorage;
		if (VK_RESOURCE()->CreateBuffer(indirectCI))
			mbRendererInit = true;

		mpGPUCullingShader = VulkanShader::Create(mpContext->device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("culling/culling", mpGPUCullingShader.get());

		mpContext->computeCommandPool->AllocateCommandBuffer(mComputeCmd);
	}

	void IndirectRenderer::OnShutdown()
	{
		mpGPUCullingShader.reset();
		mpGPUCullingPipelineSignature.reset();
		Memory::Delete(mpGPUCullingPipeline);
		mpContext->computeCommandPool->FreeCommandBuffer(mComputeCmd);
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
		indirectCommands.resize(4);
		pRenderDataBuilder->TraverseRenderData([&](UInt32 meshId, const RenderMeshBuildData& buildData)
			{
				// insert new drawcall index
				mDrawCallIndexMap[meshId] = mCurrDrawCallIndex++;

				if (buildData.instanceCount > 1) // move it to the Forward Instance Mesh Pass
				{
					const UInt64 ivbSize = sizeof(PerInstanceData) * buildData.instanceCount;
					if (mPackedVIBCurrOffset + ivbSize > SG_MAX_PACKED_INSTANCE_BUFFER_SIZE)
					{
						SG_LOG_ERROR("Instance buffer exceed boundary!");
						SG_ASSERT(false);
					}

					// create one big vertex buffer
					BufferCreateDesc vibCI = {};
					vibCI.name = "instanceBuffer";
					vibCI.bufferSize = SG_MAX_PACKED_INSTANCE_BUFFER_SIZE;
					vibCI.type = EBufferType::efVertex | EBufferType::efStorage;
					vibCI.pInitData = buildData.perInstanceData.data();
					vibCI.dataSize = static_cast<UInt32>(ivbSize);
					vibCI.dataOffset = static_cast<UInt32>(mPackedVIBCurrOffset);
					vibCI.bSubBufer = true;
					VK_RESOURCE()->CreateBuffer(vibCI, true);
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
				indirectDc.first = meshId;
				indirectDc.pIndirectBuffer = VK_RESOURCE()->GetBuffer("indirectBuffer");

				if (buildData.instanceCount == 1)
				{
					mDrawCallMap[EMeshPass::eForward].emplace_back(eastl::move(indirectDc));
				}
				else
				{
					indirectDc.drawMesh.pInstanceBuffer = VK_RESOURCE()->GetBuffer("instanceBuffer");
					indirectDc.drawMesh.instanceOffset = mPackedVIBCurrOffset;
					mPackedVIBCurrOffset += static_cast<UInt32>(sizeof(PerInstanceData) * buildData.instanceCount);
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

				indirectCommands[meshId] = eastl::move(indirect);

				mPackedVBCurrOffset += static_cast<UInt32>(vbSize);
				mPackedIBCurrOffset += static_cast<UInt32>(ibSize);
			});

		mpGPUCullingPipelineSignature = VulkanPipelineSignature::Builder(*mpContext, mpGPUCullingShader)
			.Build();

		mpGPUCullingPipeline = VulkanPipeline::Builder(mpContext->device, EPipelineType::eCompute)
			.BindSignature(mpGPUCullingPipelineSignature.get())
			.BindShader(mpGPUCullingShader.get())
			.Build();

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

	void IndirectRenderer::DoCulling()
	{
		mpContext->computeCommandPool->Reset();
		mComputeCmd.BeginRecord();
		mComputeCmd.BindPipeline(mpGPUCullingPipeline);
		mComputeCmd.BindPipelineSignature(mpGPUCullingPipelineSignature.get(), EPipelineType::eCompute);

		mComputeCmd.Dispatch(1, 1, 1);

		mComputeCmd.EndRecord();

		// temporary: should use pipeline barrier
		mpContext->computeQueue.SubmitCommands(&mComputeCmd, nullptr, nullptr, nullptr);
		mpContext->computeQueue.WaitIdle();
	}

	void IndirectRenderer::Draw(EMeshPass meshPass)
	{
		for (auto& dc : mDrawCallMap[meshPass])
		{
			BindMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(dc.pIndirectBuffer, dc.first * sizeof(DrawIndexedIndirectCommand), dc.count, sizeof(DrawIndexedIndirectCommand));
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