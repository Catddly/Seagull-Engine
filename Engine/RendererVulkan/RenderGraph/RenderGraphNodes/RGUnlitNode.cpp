#include "StdAfx.h"
#include "RGUnlitNode.h"

#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanDevice.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

namespace SG
{

	RGUnlitNode::RGUnlitNode(VulkanDevice& device)
		: mDevice(device), mpColorRt(nullptr), mpDepthRt(nullptr), 
		mpPipeline(nullptr), mpPipelineLayout(nullptr)
	{
	}

	void RGUnlitNode::BindMainRenderTarget(VulkanRenderTarget* pColorRt, const LoadStoreClearOp& op)
	{
		if (!pColorRt)
		{
			SG_LOG_ERROR("Can not pass in a nullptr!");
			return;
		}
		if (mpColorRt)
		{
			SG_LOG_ERROR("Already bind a main render target!");
			return;
		}
		mpColorRt = pColorRt;
		mColorRtLoadStoreOp = op;
	}

	void RGUnlitNode::BindMainDepthBuffer(VulkanRenderTarget* pDepthRt, const LoadStoreClearOp& op)
	{
		if (!pDepthRt)
		{
			SG_LOG_ERROR("Can not pass in a nullptr!");
			return;
		}
		if (mpDepthRt)
		{
			SG_LOG_ERROR("Already bind a main depth buffer!");
			return;
		}
		mpDepthRt = pDepthRt;
		mDepthRtLoadStoreOp = op;
	}

	void RGUnlitNode::BindPipeline(VulkanPipelineLayout* pLayout, Shader& shader)
	{
		mpPipelineLayout = pLayout;
		// TODO: use shader reflection
		BufferLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			{ EShaderDataType::eFloat3, "color" },
		};

		mpPipeline = VulkanPipeline::Builder(mDevice)
			.SetVertexLayout(vertexBufferLayout)
			.BindLayout(mpPipelineLayout)
			.BindRenderPass(mpRenderPass)
			.BindShader(&shader)
			.Build();
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
		mpRenderPass = VulkanRenderPass::Builder(mDevice)
			.BindColorRenderTarget(mpColorRt, mColorRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efPresent)
			.BindDepthRenderTarget(mpDepthRt, mDepthRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil)
			.CombineAsSubpass()
			.Build();
		return mpRenderPass;
	}

	void RGUnlitNode::Execute(VulkanCommandBuffer& pBuf)
	{
		pBuf.SetViewport((float)mpColorRt->width, (float)mpColorRt->height, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mpColorRt->width, (int)mpColorRt->height });

		pBuf.BindPipeline(mpPipeline);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pIndexBuffer = VulkanResourceRegistry::GetInstance()->GetBuffer("IndexBuffer");
		pBuf.BindVertexBuffer(0, 1, *VulkanResourceRegistry::GetInstance()->GetBuffer("VertexBuffer"), offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		for (auto& e : mDescriptorSets)
		{
			pBuf.BindDescriptorSet(mpPipelineLayout, e.first, e.second);
		}

		UInt32 pushOffset = 0;
		for (auto& e : mPushConstants)
		{
			pBuf.PushConstants(mpPipelineLayout, e.stage, e.size, pushOffset, &e.pData);
			pushOffset += e.size;
		}

		UInt32 indexCount = pIndexBuffer->SizeInByte() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

	void RGUnlitNode::Clear()
	{
		Memory::Delete(mpPipeline);
		Memory::Delete(mpRenderPass);
	}

}