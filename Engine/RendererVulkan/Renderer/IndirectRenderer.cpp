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
	UInt32 IndirectRenderer::mPackedVBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedVIBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedIBCurrOffset = 0;
	UInt32 IndirectRenderer::mCurrDrawCallIndex = 0;

	RefPtr<VulkanPipelineSignature> IndirectRenderer::mpGPUCullingPipelineSignature = nullptr;
	VulkanPipeline*                 IndirectRenderer::mpGPUCullingPipeline = nullptr;
	RefPtr<VulkanShader>            IndirectRenderer::mpGPUCullingShader = nullptr;

	RefPtr<VulkanPipelineSignature> IndirectRenderer::mpDrawCallCompactPipelineSignature = nullptr;
	VulkanPipeline*                 IndirectRenderer::mpDrawCallCompactPipeline = nullptr;
	RefPtr<VulkanShader>            IndirectRenderer::mpDrawCallCompactShader = nullptr;

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
		if (!VK_RESOURCE()->CreateBuffer(indirectCI))
			return;

		BufferCreateDesc insOutCI = {};
		insOutCI.name = "instanceOutput";
		insOutCI.bufferSize = SG_MAX_NUM_OBJECT * sizeof(InstanceOutputData);
		insOutCI.type = EBufferType::efStorage;
		if (!VK_RESOURCE()->CreateBuffer(insOutCI, true))
			return;
		mbRendererInit = true;

		mpGPUCullingShader = VulkanShader::Create(mpContext->device);
		mpDrawCallCompactShader = VulkanShader::Create(mpContext->device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("culling/culling", mpGPUCullingShader.get());
		compiler.CompileGLSLShader("culling/drawcall_compact", mpDrawCallCompactShader.get());
	}

	void IndirectRenderer::OnShutdown()
	{
		mpDrawCallCompactShader.reset();
		mpDrawCallCompactPipelineSignature.reset();
		Memory::Delete(mpDrawCallCompactPipeline);

		mpGPUCullingShader.reset();
		mpGPUCullingPipelineSignature.reset();
		Memory::Delete(mpGPUCullingPipeline);

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
		indirectCommands.resize(MeshDataArchive::GetInstance()->GetNumMeshData());
		vector<InstanceOutputData> instanceOutputData;
		instanceOutputData.emplace_back(InstanceOutputData()); // empty instance of the skybox

		pRenderDataBuilder->TraverseRenderData([&](UInt32 meshId, const RenderMeshBuildData& buildData)
			{
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

					InstanceOutputData insOutputData = {};
					insOutputData.baseOffset = mPackedVIBCurrOffset / sizeof(PerInstanceData);
					for (UInt32 i = 0; i < buildData.instanceCount; ++i)
						instanceOutputData.emplace_back(insOutputData);
				}
				else // see every draw call as one instance
				{
					instanceOutputData.emplace_back(InstanceOutputData());
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

		BufferCreateDesc insOutputCI = {};
		insOutputCI.name = "instanceOutput";
		insOutputCI.type = EBufferType::efStorage;
		insOutputCI.bufferSize = SG_MAX_NUM_OBJECT * sizeof(InstanceOutputData);
		insOutputCI.pInitData = instanceOutputData.data();
		insOutputCI.dataSize = static_cast<UInt32>(sizeof(InstanceOutputData) * instanceOutputData.size());
		insOutputCI.dataOffset = 0;
		insOutputCI.bSubBufer = true;
		VK_RESOURCE()->CreateBuffer(insOutputCI, true);
		VK_RESOURCE()->FlushBuffers();

		VK_RESOURCE()->GetBuffer("indirectBuffer")->UploadData(indirectCommands.data(), static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand)* indirectCommands.size()), 0);

		mpGPUCullingPipelineSignature = VulkanPipelineSignature::Builder(*mpContext, mpGPUCullingShader)
			.Build();

		mpGPUCullingPipeline = VulkanPipeline::Builder(mpContext->device, EPipelineType::eCompute)
			.BindSignature(mpGPUCullingPipelineSignature.get())
			.BindShader(mpGPUCullingShader.get())
			.Build();

		mpDrawCallCompactPipelineSignature = VulkanPipelineSignature::Builder(*mpContext, mpDrawCallCompactShader)
			.Build();

		mpDrawCallCompactPipeline = VulkanPipeline::Builder(mpContext->device, EPipelineType::eCompute)
			.BindSignature(mpDrawCallCompactPipelineSignature.get())
			.BindShader(mpDrawCallCompactShader.get())
			.Build();

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
		auto& cmd = mpContext->computeCmdBuffer;
		cmd.BeginRecord();

		//cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("indirectBuffer"), EPipelineStage::efIndirect_Read, EPipelineStage::efShader_Write,
		//	EPipelineType::eGraphic, EPipelineType::eCompute);
		
		// [Data Hazard] barrier to prevent instanceOutput RAW(Read After Write) scenario
		cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("instanceOutput"), EPipelineStage::efShader_Read, EPipelineStage::efShader_Write,
			EPipelineType::eCompute, EPipelineType::eCompute);

		cmd.BindPipeline(mpGPUCullingPipeline);
		cmd.BindPipelineSignature(mpGPUCullingPipelineSignature.get(), EPipelineType::eCompute);
		cmd.Dispatch(1, 1, 1);

		cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("instanceOutput"), EPipelineStage::efShader_Write, EPipelineStage::efShader_Read,
			EPipelineType::eCompute, EPipelineType::eCompute);

		cmd.BindPipeline(mpDrawCallCompactPipeline);
		cmd.BindPipelineSignature(mpDrawCallCompactPipelineSignature.get(), EPipelineType::eCompute);
		cmd.Dispatch(1, 1, 1);

		// We use semaphore to ensure that out compute command is finish before the graphic queue begin to draw,
		// So i think we don't need the buffer barrier here!
		 
		//cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("indirectBuffer"), EPipelineStage::efShader_Write, EPipelineStage::efIndirect_Read,
		//	EPipelineType::eCompute, EPipelineType::eGraphic);

		cmd.EndRecord();

		// submit compute commands
		// semaphore used to ensure the sequence of GPU-side execution
		// fence used to ensure CPU is not write to a pending status command buffer. (because here we only have one compute command buffer)
		mpContext->computeQueue.SubmitCommands(&cmd, mpContext->pComputeCompleteSemaphore, nullptr, mpContext->pComputeSyncFence);
	}

	void IndirectRenderer::Draw(EMeshPass meshPass)
	{
		for (auto& dc : mDrawCallMap[meshPass])
		{
			BindMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(dc.pIndirectBuffer, dc.first * sizeof(DrawIndexedIndirectCommand), 1, sizeof(DrawIndexedIndirectCommand));
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