#include "StdAfx.h"
#include "RGUnlitNode.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
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

	VulkanRenderPass* RGUnlitNode::Prepare()
	{
		mpRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindColorRenderTarget(mContext.colorRts[0], mColorRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efPresent)
			.BindDepthRenderTarget(mContext.depthRt, mDepthRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil)
			.CombineAsSubpass()
			.Build();

		// TODO: use shader reflection
		VertexLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "normal" },
			//{ EShaderDataType::eFloat2, "uv" },
		};

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.SetVertexLayout(vertexBufferLayout)
			.BindLayout(mpPipelineLayout)
			.BindRenderPass(mpRenderPass)
			.BindShader(mpShader)
			.Build();

		return mpRenderPass;
	}

	void RGUnlitNode::Execute(VulkanCommandBuffer& pBuf)
	{
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

	void RGUnlitNode::Clear()
	{
		Memory::Delete(mpPipeline);
		Memory::Delete(mpRenderPass);
	}

}