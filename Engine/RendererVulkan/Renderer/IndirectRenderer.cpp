#include "StdAfx.h"
#include "IndirectRenderer.h"

#include "System/System.h"
#include "Scene/Mesh/MeshDataArchive.h"
#include "Render/Shader/ShaderComiler.h"
#include "Profile/Profile.h"
//#include "Render/CommonRenderData.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"
//#include "RendererVulkan/Backend/VulkanQueryPool.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	VulkanContext* IndirectRenderer::mpContext = nullptr;
	VulkanCommandBuffer* IndirectRenderer::mpCmdBuf = nullptr;

	//VulkanQueryPool* IndirectRenderer::pComputeResetQueryPool = nullptr;
	//VulkanQueryPool* IndirectRenderer::pComputeCullingQueryPool = nullptr;

	eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> IndirectRenderer::mDrawCallMap;
	UInt32 IndirectRenderer::mPackedVBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedVIBCurrOffset = 0;
	UInt32 IndirectRenderer::mPackedIBCurrOffset = 0;
	UInt32 IndirectRenderer::mCurrDrawCallIndex = 0;

	VulkanCommandBuffer IndirectRenderer::mResetCommand;
	VulkanSemaphore* IndirectRenderer::mpWaitResetSemaphore = nullptr;

	RefPtr<VulkanPipelineSignature> IndirectRenderer::mpResetCullingPipelineSignature = nullptr;
	VulkanPipeline*                 IndirectRenderer::mpResetCullingPipeline = nullptr;
	RefPtr<VulkanShader>            IndirectRenderer::mpResetCullingShader = nullptr;

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
		SG_PROFILE_FUNCTION();

		mpContext = &context;

		BufferCreateDesc indirectCI = {};
		indirectCI.name = "indirectBuffer";
		indirectCI.bufferSize = sizeof(DrawIndexedIndirectCommand) * SG_MAX_DRAW_CALL;
		indirectCI.type = EBufferType::efIndirect | EBufferType::efStorage;
		indirectCI.memoryUsage = EGPUMemoryUsage::eGPU_To_CPU;
		indirectCI.memoryFlag = EGPUMemoryFlag::efPersistent_Map;
		if (!VK_RESOURCE()->CreateBuffer(indirectCI))
			return;

		BufferCreateDesc insOutCI = {};
		insOutCI.name = "instanceOutput";
		insOutCI.bufferSize = SG_MAX_NUM_OBJECT * sizeof(InstanceOutputData);
		insOutCI.type = EBufferType::efStorage;
		insOutCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
		if (!VK_RESOURCE()->CreateBuffer(insOutCI))
			return;
		mbRendererInit = true;

		mpGPUCullingShader = VulkanShader::Create(mpContext->device);
		mpDrawCallCompactShader = VulkanShader::Create(mpContext->device);
		mpResetCullingShader = VulkanShader::Create(mpContext->device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("culling/culling", mpGPUCullingShader.get());
		compiler.CompileGLSLShader("culling/drawcall_compact", mpDrawCallCompactShader.get());
		compiler.CompileGLSLShader("culling/culling_reset", mpResetCullingShader.get());

		mpWaitResetSemaphore = VulkanSemaphore::Create(mpContext->device);
		mpContext->computeCommandPool->AllocateCommandBuffer(mResetCommand);

		//pComputeResetQueryPool = VulkanQueryPool::Create(mpContext->device, ERenderQueryType::ePipeline_Statistics, EPipelineStageQueryType::efCompute_Shader_Invocations);
		//pComputeCullingQueryPool = VulkanQueryPool::Create(mpContext->device, ERenderQueryType::ePipeline_Statistics, EPipelineStageQueryType::efCompute_Shader_Invocations);
	}

	void IndirectRenderer::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		//Memory::Delete(pComputeResetQueryPool);
		//Memory::Delete(pComputeCullingQueryPool);

		mpResetCullingShader.reset();
		mpResetCullingPipelineSignature.reset();
		Delete(mpResetCullingPipeline);

		mpDrawCallCompactShader.reset();
		mpDrawCallCompactPipelineSignature.reset();
		Delete(mpDrawCallCompactPipeline);

		mpGPUCullingShader.reset();
		mpGPUCullingPipelineSignature.reset();
		Delete(mpGPUCullingPipeline);

		mpContext->computeCommandPool->FreeCommandBuffer(mResetCommand);
		Delete(mpWaitResetSemaphore);
		VK_RESOURCE()->DeleteBuffer("indirectBuffer");
		mbRendererInit = false;
	}

	void IndirectRenderer::CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder)
	{
		SG_PROFILE_FUNCTION();

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
					vibCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
					vibCI.pInitData = buildData.perInstanceData.data();
					vibCI.subBufferSize = static_cast<UInt32>(ivbSize);
					vibCI.subBufferOffset = static_cast<UInt32>(mPackedVIBCurrOffset);
					vibCI.bSubBuffer = true;
					VK_RESOURCE()->CreateBuffer(vibCI);
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
				vbCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
				vbCI.pInitData = pMeshData->vertices.data();
				vbCI.subBufferSize = static_cast<UInt32>(vbSize);
				vbCI.subBufferOffset = static_cast<UInt32>(mPackedVBCurrOffset);
				vbCI.bSubBuffer = true;
				VK_RESOURCE()->CreateBuffer(vbCI);

				// create one big index buffer
				BufferCreateDesc ibCI = {};
				ibCI.name = "packed_index_buffer_0";
				ibCI.bufferSize = SG_MAX_PACKED_INDEX_BUFFER_SIZE;
				ibCI.type = EBufferType::efIndex;
				ibCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
				ibCI.pInitData = pMeshData->indices.data();
				ibCI.subBufferSize = static_cast<UInt32>(ibSize);
				ibCI.subBufferOffset = static_cast<UInt32>(mPackedIBCurrOffset);
				ibCI.bSubBuffer = true;
				VK_RESOURCE()->CreateBuffer(ibCI);
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
		insOutputCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
		insOutputCI.pInitData = instanceOutputData.data();
		insOutputCI.subBufferSize = static_cast<UInt32>(sizeof(InstanceOutputData) * instanceOutputData.size());
		insOutputCI.subBufferOffset = 0;
		insOutputCI.bSubBuffer = true;
		VK_RESOURCE()->CreateBuffer(insOutputCI);
		VK_RESOURCE()->FlushBuffers();

		VK_RESOURCE()->GetBuffer("indirectBuffer")->UploadData(indirectCommands.data(), static_cast<UInt32>(sizeof(DrawIndexedIndirectCommand)* indirectCommands.size()), 0);

		mpResetCullingPipelineSignature = VulkanPipelineSignature::Builder(*mpContext, mpResetCullingShader)
			.Build();

		mpResetCullingPipeline = VulkanPipeline::Builder(mpContext->device, EPipelineType::eCompute)
			.BindSignature(mpResetCullingPipelineSignature.get())
			.BindShader(mpResetCullingShader.get())
			.Build();

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
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

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
		// [Critical] Here discovered severe performance fluctuation.
		SG_PROFILE_FUNCTION();

		{
			SG_PROFILE_SCOPE("Compute Culling Reset");

			// Why adding a query pool will fail the submittion of the command?
			//auto& statistics = GetStatisticData();

			mResetCommand.Reset();
			mResetCommand.BeginRecord();
			{
				//mResetCommand.ResetQueryPool(pComputeResetQueryPool);
				//mResetCommand.BeginQuery(pComputeResetQueryPool, 0);

				mResetCommand.BindPipeline(mpResetCullingPipeline);
				mResetCommand.BindPipelineSignature(mpResetCullingPipelineSignature.get(), EPipelineType::eCompute);
				UInt32 numGroup = (UInt32)(MeshDataArchive::GetInstance()->GetNumMeshData() / 16) + 1;
				mResetCommand.Dispatch(numGroup, 1, 1);

				//mResetCommand.EndQuery(pComputeResetQueryPool, 0);
			}
			mResetCommand.EndRecord();

			mpContext->computeQueue.SubmitCommands(&mResetCommand, mpWaitResetSemaphore, nullptr, nullptr);
			//statistics.pipelineStatistics[6] = *pComputeResetQueryPool->GetQueryResult(0, 1);
		}

		{
			SG_PROFILE_SCOPE("Compute Culling");

			auto& cmd = mpContext->computeCmdBuffer;
			cmd.BeginRecord();
			{
				//cmd.ResetQueryPool(pComputeCullingQueryPool);
				//cmd.BeginQuery(pComputeCullingQueryPool, 0);
				//cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("indirectBuffer"), EPipelineStage::efIndirect_Read, EPipelineStage::efShader_Write,
				//	EPipelineType::eGraphic, EPipelineType::eCompute);

				// [Data Hazard] barrier to prevent instanceOutput RAW(Read After Write) scenario
				cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("instanceOutput"), EPipelineStageAccess::efShader_Read, EPipelineStageAccess::efShader_Write,
					EPipelineType::eCompute, EPipelineType::eCompute);

				cmd.BindPipeline(mpGPUCullingPipeline);
				cmd.BindPipelineSignature(mpGPUCullingPipelineSignature.get(), EPipelineType::eCompute);
				UInt32 numGroup = (UInt32)(SSystem()->GetMainScene()->GetMeshEntityCount() / 128) + 1;
				cmd.Dispatch(numGroup, 1, 1);

				cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("instanceOutput"), EPipelineStageAccess::efShader_Write, EPipelineStageAccess::efShader_Read,
					EPipelineType::eCompute, EPipelineType::eCompute);

				cmd.BindPipeline(mpDrawCallCompactPipeline);
				cmd.BindPipelineSignature(mpDrawCallCompactPipelineSignature.get(), EPipelineType::eCompute);
				cmd.Dispatch(1, 1, 1);

				// We use semaphore to ensure that out compute command is finish before the graphic queue begin to draw,
				// So i think we don't need the buffer barrier here!
		 
				//cmd.BufferBarrier(VK_RESOURCE()->GetBuffer("indirectBuffer"), EPipelineStage::efShader_Write, EPipelineStage::efIndirect_Read,
				//	EPipelineType::eCompute, EPipelineType::eGraphic);
				//cmd.EndQuery(pComputeCullingQueryPool, 0);
			}
			cmd.EndRecord();

			// submit compute commands
			// semaphore used to ensure the sequence of GPU-side execution
			// fence used to ensure CPU is not write to a pending status command buffer. (because here we only have one compute command buffer)
			mpContext->computeQueue.SubmitCommands(&cmd, mpContext->pComputeCompleteSemaphore, mpWaitResetSemaphore, mpContext->pComputeSyncFence);
			//statistics.pipelineStatistics[7] = *pComputeCullingQueryPool->GetQueryResult(0, 1);
		}
	}

	void IndirectRenderer::Draw(EMeshPass meshPass)
	{
		SG_PROFILE_FUNCTION();

		for (auto& dc : mDrawCallMap[meshPass])
		{
			BindMesh(dc.drawMesh);
			BindMaterial(dc.drawMaterial);

			mpCmdBuf->DrawIndexedIndirect(dc.pIndirectBuffer, dc.first * sizeof(DrawIndexedIndirectCommand), 1, sizeof(DrawIndexedIndirectCommand));
		}
	}

	void IndirectRenderer::BindMesh(const DrawMesh& drawMesh)
	{
		SG_PROFILE_FUNCTION();

		mpCmdBuf->BindVertexBuffer(0, 1, *drawMesh.pVertexBuffer, &drawMesh.vBOffset);
		if (drawMesh.pInstanceBuffer)
			mpCmdBuf->BindVertexBuffer(1, 1, *drawMesh.pInstanceBuffer, &drawMesh.instanceOffset);
		mpCmdBuf->BindIndexBuffer(*drawMesh.pIndexBuffer, drawMesh.iBOffset);
	}

	void IndirectRenderer::BindMaterial(const DrawMaterial& drawMaterial)
	{

	}

}