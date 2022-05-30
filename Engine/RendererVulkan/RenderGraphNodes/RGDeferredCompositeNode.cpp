#include "StdAfx.h"
#include "RGDeferredCompositeNode.h"

#include "Profile/Profile.h"
#include "Render/Shader/ShaderComiler.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanTexture.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#if SG_ENABLE_DEFERRED_SHADING
namespace SG
{

	RGDeferredCompositeNode::RGDeferredCompositeNode(VulkanContext& context, RenderGraph* pRenderGraph)
		:RenderGraphNode(pRenderGraph), mContext(context),
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		SG_PROFILE_FUNCTION();

		mpGBufferCompositeShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("deferred/deferred_pbr_shading", mpGBufferCompositeShader);

		mpSkyboxShader = VulkanShader::Create(mContext.device);
		compiler.CompileGLSLShader("skybox", mpSkyboxShader);

		mpSkyboxPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpSkyboxShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "deferred_rt_sampler";
		samplerCI.filterMode = EFilterMode::eNearest;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 1.0f;
		samplerCI.enableAnisotropy = true;
		samplerCI.maxAnisotropy = 1.0f;
		VK_RESOURCE()->CreateSampler(samplerCI);

		mpGBufferCompositePipelineSignature = VulkanPipelineSignature::Builder(mContext, mpGBufferCompositeShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.AddCombindSamplerImage("brdf_lut_sampler", "brdf_lut")
			.AddCombindSamplerImage("irradiance_cubemap_sampler", "cubemap_irradiance")
			.AddCombindSamplerImage("prefilter_cubemap_sampler", "cubemap_prefilter")
			.AddCombindSamplerImage("deferred_rt_sampler", "position_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "normal_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "albedo_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "mrao_deferred_rt")
			.Build();

#ifdef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#endif
	}

	RGDeferredCompositeNode::~RGDeferredCompositeNode()
	{
		SG_PROFILE_FUNCTION();

		Delete(mpSkyboxPipeline);
		Delete(mpGBufferCompositePipeline);
	}

	void RGDeferredCompositeNode::Reset()
	{
		SG_PROFILE_FUNCTION();

		VulkanPipelineSignature::ShaderDataBinder(mpGBufferCompositePipelineSignature.get(), mpGBufferCompositeShader, 1)
			.AddCombindSamplerImage("deferred_rt_sampler", "position_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "normal_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "albedo_deferred_rt")
			.AddCombindSamplerImage("deferred_rt_sampler", "mrao_deferred_rt")
			.ReBind();

#ifdef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#endif
	}

	void RGDeferredCompositeNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		SG_PROFILE_FUNCTION();

		mpSkyboxPipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pRenderpass)
			.BindSignature(mpSkyboxPipelineSignature)
			.SetRasterizer(ECullMode::eFront)
			.SetColorBlend(false)
			.SetDynamicStates()
			.Build();

		mpGBufferCompositePipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pRenderpass)
			.BindSignature(mpGBufferCompositePipelineSignature)
			.SetRasterizer(ECullMode::eNone)
			.SetColorBlend(false)
			.SetDynamicStates()
			.SetDepthStencil(false)
			.Build();
	}

	void RGDeferredCompositeNode::Draw(DrawInfo& context)
	{
		SG_PROFILE_FUNCTION();

		auto& pBuf = *context.pCmd;

		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set min z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignatureNonDynamic(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
			pBuf.BindVertexBuffer(0, 1, *skybox.drawMesh.pVertexBuffer, &skybox.drawMesh.vBOffset);
			pBuf.Draw(36, 1, 0, 0);
		}

		// 2. Composite G-Buffers
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
			
			pBuf.BindPipeline(mpGBufferCompositePipeline);
			pBuf.BindPipelineSignatureNonDynamic(mpGBufferCompositePipelineSignature.get());

			pBuf.Draw(3, 1, 0, 0);
		}
	}

}
#endif // SG_ENABLE_DEFERRED_SHADING