#include "StdAfx.h"
#include "RGShadowNode.h"

#include "System/System.h"
#include "Render/CommonRenderData.h"
#include "Render/Shader/ShaderComiler.h"
#include "Profile/Profile.h"

#include "Math/MathBasic.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanQueue.h"
#include "RendererVulkan/Backend/VulkanTexture.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "RendererVulkan/RenderDevice/DrawCall.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/DrawProtocol/ForwardDP.h"
#include "RendererVulkan/DrawProtocol/GPUDrivenDP.h"

namespace SG
{

#define SG_SHADOW_MAP_SIZE 2048

	RGShadowNode::RGShadowNode(VulkanContext& context, RenderGraph* pRenderGraph)
		:RenderGraphNode(pRenderGraph), mContext(context),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		SG_PROFILE_FUNCTION();

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
		mContext.pGraphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("shadow map"), EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only);
		pCmd.EndRecord();
		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&pCmd, nullptr, nullptr, nullptr, nullptr);
		mContext.pGraphicQueue->WaitIdle();
		mContext.pGraphicCommandPool->FreeCommandBuffer(pCmd);

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
		compiler.CompileGLSLShader("shadow", mpShadowShader);
		compiler.CompileGLSLShader("shadow_instance", mpShadowInstanceShader);

		mpShadowPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShadowShader)
			.Build();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	RGShadowNode::~RGShadowNode()
	{
		SG_PROFILE_FUNCTION();

		Delete(mpShadowInstancePipeline);
		Delete(mpShadowPipeline);
	}

	void RGShadowNode::Reset()
	{
		SG_PROFILE_FUNCTION();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	void RGShadowNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		SG_PROFILE_FUNCTION();

		mpShadowInstancePipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pRenderpass)
			.BindSignature(mpShadowPipelineSignature, true)
			.BindShader(mpShadowInstanceShader)
			.SetInputVertexRange(sizeof(Vertex), EVertexInputRate::ePerVertex)
			.SetInputVertexRange(sizeof(PerInstanceData), EVertexInputRate::ePerInstance)
			.SetDepthStencil(true)
			.SetColorBlend(false)
			.SetRasterizer(ECullMode::eBack)
			.SetDynamicStates(VK_DYNAMIC_STATE_DEPTH_BIAS)
			.Build();

		mpShadowPipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pRenderpass)
			.BindSignature(mpShadowPipelineSignature, true)
			.BindShader(mpShadowShader)
			.SetDepthStencil(true)
			.SetColorBlend(false)
			.SetRasterizer(ECullMode::eBack)
			.SetDynamicStates(VK_DYNAMIC_STATE_DEPTH_BIAS)
			.Build();
	}

	void RGShadowNode::Draw(DrawInfo& context)
	{
		SG_PROFILE_FUNCTION();

		GPUDrivenDP::Begin(context);
#if SG_ENABLE_GPU_CULLING
		GPUDrivenDP::CullingReset();
		GPUDrivenDP::DoCulling();
		GPUDrivenDP::CopyStatisticsData();
#endif

		auto& pBuf = *context.pCmd;
		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efTop_Of_Pipeline, 0);

		pBuf.SetViewport(SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE });

		pBuf.SetDepthBias(1.5f, 0.0f, 1.75f);

		if (mbDrawShadow)
		{
			pBuf.BindPipelineSignatureNonDynamic(mpShadowPipelineSignature.get());

			// 1.1 Forward Mesh Pass
			pBuf.BindPipeline(mpShadowPipeline);
			GPUDrivenDP::DrawWithoutBindMaterial(EMeshPass::eForward);

			// 1.2 Forward Instanced Mesh Pass
			pBuf.BindPipeline(mpShadowInstancePipeline);
			GPUDrivenDP::DrawWithoutBindMaterial(EMeshPass::eForwardInstanced);
		}
		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efBottom_Of_Pipeline, 1);

		GPUDrivenDP::End();
	}

}