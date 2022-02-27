#include "StdAfx.h"
#include "RGUnlitNode.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "RendererVulkan/Resource/Geometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	RGUnlitNode::RGUnlitNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr), mpPipelineLayout(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
	}

	RGUnlitNode::~RGUnlitNode()
	{
		Memory::Delete(mpPipeline);
	}

	void RGUnlitNode::BindGeometry(const char* name)
	{
		mpGeometry = VK_RESOURCE()->GetGeometry(name);
		if (!mpGeometry)
		{
			SG_LOG_ERROR("Failed to bind geometry! Geometry does not exist!");
			SG_ASSERT(false);
		}
	}

	void RGUnlitNode::BindPipeline(VulkanPipelineLayout* pLayout, Shader* pShader)
	{
		mpPipelineLayout = pLayout;
		mpShader = pShader;
	}

	void RGUnlitNode::AddDescriptorSet(UInt32 set, VkDescriptorSet handle)
	{
		mDescriptorSets.push_back({ set, handle });
	}

	void RGUnlitNode::AddConstantBuffer(EShaderStage stage, UInt32 size, void* pData)
	{
		mPushConstants.push_back({ stage, size, pData });
	}

	void RGUnlitNode::Update(UInt32 frameIndex)
	{
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp });
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp });
	}

	void RGUnlitNode::Reset()
	{
		ClearResources();
		mbDepthUpdated = false;
	}

	void RGUnlitNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		// TODO: use shader reflection
		VertexLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			//{ EShaderDataType::eFloat3, "color" },
			{ EShaderDataType::eFloat3, "normal" },
			//{ EShaderDataType::eFloat2, "uv" },
		};

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.SetVertexLayout(vertexBufferLayout)
			.BindLayout(mpPipelineLayout)
			.BindRenderPass(pRenderpass)
			.BindShader(mpShader)
			.Build();
	}

	void RGUnlitNode::Execute(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		for (auto& e : mDescriptorSets)
			pBuf.BindDescriptorSet(mpPipelineLayout, e.first, e.second);
		pBuf.BindPipeline(mpPipeline);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer  = mpGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		UInt32 pushOffset = 0;
		for (auto& e : mPushConstants)
		{
			pBuf.PushConstants(mpPipelineLayout, e.stage, e.size, pushOffset, e.pData);
			pushOffset += e.size;
			UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
			pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
		}
	}

}