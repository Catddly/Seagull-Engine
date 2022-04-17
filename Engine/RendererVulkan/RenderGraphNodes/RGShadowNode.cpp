#include "StdAfx.h"
#include "RGShadowNode.h"

#include "System/System.h"
#include "Render/CommonRenderData.h"
#include "Render/Shader/ShaderComiler.h"

#include "Math/MathBasic.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/Renderer/Renderer.h"
#include "RendererVulkan/Renderer/IndirectRenderer.h"

namespace SG
{

#define SG_SHADOW_MAP_SIZE 2048

	RGShadowNode::RGShadowNode(VulkanContext& context, RenderGraph* pRenderGraph)
		:RenderGraphNode(pRenderGraph), mContext(context),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		mbDrawShadow = true;

		TextureCreateDesc texCI = {};
		texCI.name = "shadow map";
		texCI.width = SG_SHADOW_MAP_SIZE;
		texCI.height = SG_SHADOW_MAP_SIZE;
		texCI.depth = 1;
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.usage = EImageUsage::efDepth_Stencil | EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.sample = ESampleCount::eSample_1;
		texCI.format = EImageFormat::eUnorm_D16;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateRenderTarget(texCI, true);

		//! [Critical] 
		//! This shadow map is used by DrawSceneNode as shader resource, so we need to transit the image layout here.
		//! If we create the pipeline signature after the execution of the this ShadowNode, we don't need to do a transition here.
		//! So this is for consistency of the architecture.
		//! Two ways, both are OK.
		VulkanCommandBuffer pCmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("shadow map"), EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "shadow_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		mpShadowShader = VulkanShader::Create(mContext.device);
		mpShadowInstanceShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("shadow", mpShadowShader.get());
		compiler.CompileGLSLShader("shadow_instance", mpShadowInstanceShader.get());

		mpShadowPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShadowShader)
			.Build();
		mpShadowInstancePipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShadowInstanceShader)
			.Build();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	RGShadowNode::~RGShadowNode()
	{
		Memory::Delete(mpShadowInstancePipeline);
		Memory::Delete(mpShadowPipeline);
	}

	void RGShadowNode::Reset()
	{
		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	void RGShadowNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpShadowInstancePipeline = VulkanPipeline::Builder(mContext.device)
			.SetInputVertexRange(sizeof(Vertex), EVertexInputRate::ePerVertex)
			.SetInputVertexRange(sizeof(PerInstanceData), EVertexInputRate::ePerInstance)
			.SetDepthStencil(true)
			.SetColorBlend(false)
			.SetRasterizer(ECullMode::eFront)
			.SetDynamicStates(VK_DYNAMIC_STATE_DEPTH_BIAS)
			.BindSignature(mpShadowInstancePipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpShadowInstanceShader.get())
			.Build();

		mpShadowPipeline = VulkanPipeline::Builder(mContext.device)
			.SetDepthStencil(true)
			.SetColorBlend(false)
			.SetRasterizer(ECullMode::eFront)
			.SetDynamicStates(VK_DYNAMIC_STATE_DEPTH_BIAS)
			.BindSignature(mpShadowPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpShadowShader.get())
			.Build();
	}

	void RGShadowNode::Draw(RGDrawInfo& context)
	{
		// This will cull out the shadow!
		IndirectRenderer::DoCulling();

		auto& pBuf = *context.pCmd;

		pBuf.SetViewport(SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE });

		pBuf.SetDepthBias(1.5f, 0.0f, 1.75f);

		if (mbDrawShadow)
		{
			IndirectRenderer::Begin(&pBuf);
			// 1.1 Forward Mesh Pass
			pBuf.BindPipeline(mpShadowPipeline);
			pBuf.BindPipelineSignature(mpShadowPipelineSignature.get());
			IndirectRenderer::Draw(EMeshPass::eForward);

			// 1.2 Forward Instanced Mesh Pass
			pBuf.BindPipeline(mpShadowInstancePipeline);
			pBuf.BindPipelineSignature(mpShadowInstancePipelineSignature.get());
			IndirectRenderer::Draw(EMeshPass::eForwardInstanced);
			IndirectRenderer::End();
		}
	}

}