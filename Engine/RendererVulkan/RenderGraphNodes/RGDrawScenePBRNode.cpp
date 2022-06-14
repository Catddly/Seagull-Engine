#include "StdAfx.h"
#include "RGDrawScenePBRNode.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Render/Shader/ShaderComiler.h"
#include "Render/CommonRenderData.h"
#include "Archive/ResourceLoader.h"
#include "Profile/Profile.h"

#include "Stl/Utility.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"
#include "RendererVulkan/Backend/VulkanQueue.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanTexture.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/RenderDevice/DrawCall.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/DrawProtocol/ForwardDP.h"
#include "RendererVulkan/DrawProtocol/GPUDrivenDP.h"

namespace SG
{

#define IRRADIANCE_CUBEMAP_TEX_SIZE 64
#define PREFILTER_CUBEMAP_TEX_SIZE  512
#define BRDF_LUT_TEX_SIZE 512

	RGDrawScenePBRNode::RGDrawScenePBRNode(VulkanContext& context, RenderGraph* pRenderGraph)
		:RenderGraphNode(pRenderGraph), mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eClear, EStoreOp::eStore })
	{
		SG_PROFILE_FUNCTION();

		// init render resource
#ifdef SG_ENABLE_HDR
		CreateColorRt();
#endif

#if SG_ENABLE_DEFERRED_SHADING
		CreateDeferredShadingRts();
#endif

		LoadAndGenerateCubemapResource();

		mpShader = VulkanShader::Create(mContext.device);
		mpInstanceShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
#if SG_ENABLE_DEFERRED_SHADING
		compiler.CompileGLSLShader("deferred/deferred_sample", mpShader);
		compiler.CompileGLSLShader("deferred/deferred_sample_instance", "deferred/deferred_sample", mpInstanceShader);
#else
		compiler.CompileGLSLShader("brdf", mpShader);
		compiler.CompileGLSLShader("brdf_instance", "brdf", mpInstanceShader);
#endif

#if !SG_ENABLE_DEFERRED_SHADING
		mpSkyboxShader = VulkanShader::Create(mContext.device);
		compiler.CompileGLSLShader("skybox", mpSkyboxShader);

		mpSkyboxPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpSkyboxShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();
#endif

#if SG_ENABLE_DEFERRED_SHADING
		// no shadow map and others here, deferred to composite render pass
		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.Build();
#else
		// just bind set 0 resource here.
		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.AddCombindSamplerImage("brdf_lut_sampler", "brdf_lut")
			.AddCombindSamplerImage("irradiance_cubemap_sampler", "cubemap_irradiance")
			.AddCombindSamplerImage("prefilter_cubemap_sampler", "cubemap_prefilter")
			.Build();
#endif

		// TEMPORARY
		mContext.pTempPipelineSignature = mpPipelineSignature.get();
		mContext.pShader = mpShader;

#if SG_ENABLE_DEFERRED_SHADING
		ClearValue cv = {};
		cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("position_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(1, { VK_RESOURCE()->GetRenderTarget("normal_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(2, { VK_RESOURCE()->GetRenderTarget("albedo_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(3, { VK_RESOURCE()->GetRenderTarget("mrao_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });

		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(4, { mContext.depthRt, mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil });
#else
#	ifdef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#	endif
		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil });
#endif
	}

	RGDrawScenePBRNode::~RGDrawScenePBRNode()
	{
		SG_PROFILE_FUNCTION();

#if !SG_ENABLE_DEFERRED_SHADING
		Delete(mpSkyboxPipeline);
#endif
		Delete(mpInstancePipeline);
		Delete(mpPipeline);
	}

	void RGDrawScenePBRNode::Update()
	{
		SG_PROFILE_FUNCTION();

		//mMessageBusMember.ListenFor("RenderDataRebuild", SG_BIND_MEMBER_FUNC(OnRenderDataRebuild));
	}

	void RGDrawScenePBRNode::Reset()
	{
		SG_PROFILE_FUNCTION();

#if SG_ENABLE_DEFERRED_SHADING
		DestroyColorRt();
		CreateColorRt();

		DestroyDeferredShadingRts();
		CreateDeferredShadingRts();

		ClearValue cv = {};
		cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("position_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(1, { VK_RESOURCE()->GetRenderTarget("normal_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(2, { VK_RESOURCE()->GetRenderTarget("albedo_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
		AttachResource(3, { VK_RESOURCE()->GetRenderTarget("mrao_deferred_rt"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });

		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(4, { mContext.depthRt, mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil });
#else
#ifdef SG_ENABLE_HDR
		DestroyColorRt();
		CreateColorRt();

		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#endif

		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil });
#endif
	}

	void RGDrawScenePBRNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		SG_PROFILE_FUNCTION();

#if !SG_ENABLE_DEFERRED_SHADING
		mpSkyboxPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpSkyboxPipelineSignature)
			.BindRenderPass(pRenderpass)
			.SetRasterizer(ECullMode::eFront)
			.SetColorBlend(false)
			.SetDynamicStates()
			.Build();
#endif

		mpInstancePipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpPipelineSignature, true)
			.BindShader(mpInstanceShader)
			.BindRenderPass(pRenderpass)
			.SetRasterizer(ECullMode::eBack)
			.SetInputVertexRange(sizeof(Vertex), EVertexInputRate::ePerVertex)
			.SetInputVertexRange(sizeof(PerInstanceData), EVertexInputRate::ePerInstance)
#if SG_ENABLE_DEFERRED_SHADING
			.SetColorBlend(false) // for position_deferred_rt
			.SetColorBlend(false) // for normal_deferred_rt
			.SetColorBlend(false) // for albedo_deferred_rt
			.SetColorBlend(false) // for mrao_deferred_rt
#else
			.SetColorBlend(false)
#endif
			.SetDynamicStates()
			.Build();

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpPipelineSignature, true)
			.BindShader(mpShader)
			.BindRenderPass(pRenderpass)
#if SG_ENABLE_DEFERRED_SHADING
			.SetColorBlend(false) // for position_deferred_rt
			.SetColorBlend(false) // for normal_deferred_rt
			.SetColorBlend(false) // for albedo_deferred_rt
			.SetColorBlend(false) // for mrao_deferred_rt
#else
			.SetColorBlend(false)
#endif
			.SetDynamicStates()
			.Build();
	}

	void RGDrawScenePBRNode::Draw(DrawInfo& context)
	{
		SG_PROFILE_FUNCTION();

		auto& pBuf = *context.pCmd;
		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efTop_Of_Pipeline, 2);

#if !SG_ENABLE_DEFERRED_SHADING
		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignatureNonDynamic(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
			pBuf.BindVertexBuffer(0, 1, *skybox.drawMesh.pVertexBuffer, &skybox.drawMesh.vBOffset);
			pBuf.Draw(36, 1, 0, 0);
		}
#endif

		// 2. Draw Scene
		pBuf.BeginQuery(mContext.pPipelineStatisticsQueryPool, 0);
		DrawScene(context);
		pBuf.EndQuery(mContext.pPipelineStatisticsQueryPool, 0);

		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efBottom_Of_Pipeline, 3);
	}

	void RGDrawScenePBRNode::DrawScene(DrawInfo& drawInfo)
	{
		SG_PROFILE_FUNCTION();

		auto& pBuf = *drawInfo.pCmd;
		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		GPUDrivenDP::Begin(drawInfo);

		// 1.1 Forward Mesh Pass
		pBuf.BindPipeline(mpPipeline);
		GPUDrivenDP::Draw(EMeshPass::eForward);

		// 1.2 Forward Instanced Mesh Pass
		pBuf.BindPipeline(mpInstancePipeline);
		GPUDrivenDP::Draw(EMeshPass::eForwardInstanced);
		GPUDrivenDP::End();
	}

	void RGDrawScenePBRNode::GenerateBRDFLut()
	{
		SG_PROFILE_FUNCTION();

		const UInt32 texSize = BRDF_LUT_TEX_SIZE;

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "brdf_lut_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 1.0f;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		TextureCreateDesc texCI = {};
		texCI.name = "brdf_lut";
		texCI.width = texSize;
		texCI.height = texSize;
		texCI.depth = 1;
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.format = EImageFormat::eSfloat_R16G16;
		texCI.sample = ESampleCount::eSample_1;
		texCI.usage = EImageUsage::efSample | EImageUsage::efColor;
		texCI.type = EImageType::e2D;
		texCI.pInitData = nullptr;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateRenderTarget(texCI);

		auto pBrdfLutShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("composition", "generate_brdf_lut", pBrdfLutShader);

		auto pBrdfLutPipelineSignature = VulkanPipelineSignature::Builder(mContext, pBrdfLutShader)
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("brdf_lut"), mColorRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource)
			.CombineAsSubpass()
			.Build();

		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.BindRenderPass(pTempVulkanRenderPass)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("brdf_lut"))
			.Build();

		auto* pBrdfLutPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(pBrdfLutPipelineSignature)
			.BindRenderPass(pTempVulkanRenderPass)
			.SetRasterizer(ECullMode::eNone)
			.SetColorBlend(false)
			.SetDepthStencil(false)
			.SetDynamicStates()
			.Build();

		VulkanFence* pFence = VulkanFence::Create(mContext.device);
		VulkanCommandBuffer cmdBuf;
		mContext.pGraphicCommandPool->AllocateCommandBuffer(cmdBuf);

		cmdBuf.BeginRecord();
		ClearValue cv;
		cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdBuf.BeginRenderPass(pTempFrameBuffer, &cv, 1);
		{
			cmdBuf.SetViewport((float)texSize, (float)texSize, 0.0f, 1.0f);
			cmdBuf.SetScissor({ 0, 0, (int)texSize, (int)texSize });

			cmdBuf.BindPipeline(pBrdfLutPipeline);
			// draw screen triangle
			cmdBuf.Draw(3, 1, 0, 0);
		}
		cmdBuf.EndRenderPass();
		// why it is not be implicitly transition by the renderpass?
		cmdBuf.ImageBarrier(VK_RESOURCE()->GetRenderTarget("brdf_lut"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		cmdBuf.EndRecord();

		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&cmdBuf, nullptr, nullptr, nullptr, pFence);

		pFence->Wait();
		mContext.pGraphicCommandPool->FreeCommandBuffer(cmdBuf);
		Delete(pBrdfLutPipeline);
		Delete(pTempFrameBuffer);
		Delete(pTempVulkanRenderPass);
		Delete(pFence);
	}

	void RGDrawScenePBRNode::PreCalcIrradianceCubemap()
	{
		SG_PROFILE_FUNCTION();

		const UInt32 texSize = IRRADIANCE_CUBEMAP_TEX_SIZE;
		const UInt32 numMip = CalcMipmapLevel(texSize, texSize);

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "irradiance_cubemap_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)numMip;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		// create cubemap for irradiance texture
		TextureCreateDesc texCI = {};
		texCI.name = "cubemap_irradiance";
		texCI.width = texSize;
		texCI.height = texSize;
		texCI.depth = 1;
		texCI.array = 6;
		texCI.mipLevel = numMip;
		texCI.format = EImageFormat::eSfloat_R32G32B32A32;
		texCI.sample = ESampleCount::eSample_1;
		texCI.usage = EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.pInitData = nullptr;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateTexture(texCI);

		// create for drawing and copying
		texCI.name = "cubemap_irradiance_rt";
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.usage = EImageUsage::efColor | EImageUsage::efTransfer_Src;
		VK_RESOURCE()->CreateRenderTarget(texCI);

		VulkanFence* pFence = VulkanFence::Create(mContext.device);

		VulkanCommandBuffer cmd;
		mContext.pGraphicCommandPool->AllocateCommandBuffer(cmd);
		cmd.BeginRecord();
		cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_irradiance"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		cmd.EndRecord();
		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&cmd, nullptr, nullptr, nullptr, pFence);
		pFence->WaitAndReset();

		auto pIrradianceShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("irradiance_cubemap", pIrradianceShader);

		auto pIrradiancePipelineSignature = VulkanPipelineSignature::Builder(mContext, pIrradianceShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), mColorRtLoadStoreOp, EResourceBarrier::efRenderTarget, EResourceBarrier::efCopy_Source)
			.CombineAsSubpass()
			.Build();

		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"))
			.BindRenderPass(pTempVulkanRenderPass)
			.Build();

		auto* pIrradiancePipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pTempVulkanRenderPass)
			.BindSignature(pIrradiancePipelineSignature)
			.SetRasterizer(ECullMode::eNone)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.Build();

		// matrixs to rotate the corresponding faces of the cube to the right place.
		eastl::array<Matrix4f, 6> directionMats = {
			// +X
			glm::toMat4(Quaternion(Vector3f(0.0f, glm::radians(-90.0f), 0.0f))) * glm::scale(Matrix4f(1.0f), { -1.0f, 1.0f, 1.0f }),
			// -X
			glm::toMat4(Quaternion(Vector3f(0.0f, glm::radians(90.0f), 0.0f))) * glm::scale(Matrix4f(1.0f), { -1.0f, 1.0f, 1.0f }),
			// +Y
			glm::toMat4(Quaternion(Vector3f(glm::radians(90.0f), 0.0f, 0.0f))) * glm::scale(Matrix4f(1.0f), { 1.0f, -1.0f, 1.0f }),
			// -Y
			glm::toMat4(Quaternion(Vector3f(glm::radians(-90.0f), 0.0f, 0.0f))) * glm::scale(Matrix4f(1.0f), { 1.0f, -1.0f, 1.0f }),
			// +Z
			glm::scale(Matrix4f(1.0f), { 1.0f, 1.0f, -1.0f }),
			// -Z
			glm::toMat4(Quaternion(Vector3f(glm::radians(180.0f), 0.0f, glm::radians(180.0f)))) * glm::scale(Matrix4f(1.0f), { 1.0f, 1.0f, -1.0f })
		};

		struct PushConstant
		{
			Matrix4f mvp;
			float    deltaPhi;
			float    deltaTheta;
		};

		cmd.Reset();
		cmd.BeginRecord();
		{
			for (UInt32 mip = 0; mip < numMip; ++mip)
			{
				for (UInt32 face = 0; face < 6; ++face)
				{
					cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efCopy_Source, EResourceBarrier::efRenderTarget);

					ClearValue cv;
					cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
					cmd.BeginRenderPass(pTempFrameBuffer, &cv, 1);
					{
						cmd.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmd.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmd.BindPipelineSignatureNonDynamic(pIrradiancePipelineSignature.get());
						cmd.BindPipeline(pIrradiancePipeline);

						PushConstant pushConstant;
						pushConstant.mvp = BuildPerspectiveMatrix(glm::radians(90.0f), 1.0f, 0.1f, 256.0f) * directionMats[face];
						pushConstant.deltaPhi = (2.0f * float(PI)) / 180.0f;
						pushConstant.deltaTheta = (0.5f * float(PI)) / 64.0f;
						cmd.PushConstants(pIrradiancePipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &pushConstant);

						const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
						cmd.BindVertexBuffer(0, 1, *skybox.drawMesh.pVertexBuffer, &skybox.drawMesh.vBOffset);
						cmd.Draw(36, 1, 0, 0);
					}
					cmd.EndRenderPass();

					TextureCopyRegion copyRegion = {};
					copyRegion.baseArray = face;
					copyRegion.width = (texSize >> mip);
					copyRegion.height = (texSize >> mip);
					copyRegion.depth = 1;
					copyRegion.layer = 1;
					copyRegion.offset = 0;
					copyRegion.mipLevel = mip;
					cmd.CopyImage(*VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"),
						*VK_RESOURCE()->GetTexture("cubemap_irradiance"), copyRegion);
				}
			}
		}
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_irradiance"), EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
		cmd.EndRecord();

		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&cmd, nullptr, nullptr, nullptr, pFence);

		pFence->Wait();

		mContext.pGraphicCommandPool->FreeCommandBuffer(cmd);
		Delete(pIrradiancePipeline);
		Delete(pTempFrameBuffer);
		Delete(pTempVulkanRenderPass);
		Delete(pFence);
		VK_RESOURCE()->DeleteRenderTarget("cubemap_irradiance_rt");
	}

	void RGDrawScenePBRNode::PrefilterCubemap()
	{
		SG_PROFILE_FUNCTION();

		const UInt32 texSize = PREFILTER_CUBEMAP_TEX_SIZE;
		const UInt32 numMip = CalcMipmapLevel(texSize, texSize);

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "prefilter_cubemap_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)numMip;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		// create cubemap for prefilter texture
		TextureCreateDesc texCI = {};
		texCI.name = "cubemap_prefilter";
		texCI.width = texSize;
		texCI.height = texSize;
		texCI.depth = 1;
		texCI.array = 6;
		texCI.mipLevel = numMip;
		texCI.format = EImageFormat::eSfloat_R32G32B32A32;
		texCI.sample = ESampleCount::eSample_1;
		texCI.usage = EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.pInitData = nullptr;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateTexture(texCI);

		// create for drawing and copying
		texCI.name = "cubemap_prefilter_rt";
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.usage = EImageUsage::efColor | EImageUsage::efTransfer_Src;
		VK_RESOURCE()->CreateRenderTarget(texCI);

		VulkanFence* pFence = VulkanFence::Create(mContext.device);

		VulkanCommandBuffer cmd;
		mContext.pGraphicCommandPool->AllocateCommandBuffer(cmd);
		cmd.BeginRecord();
		cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_prefilter"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		cmd.EndRecord();
		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&cmd, nullptr, nullptr, nullptr, pFence);
		pFence->WaitAndReset();

		auto pPrefilterShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("prefilter_cubemap", pPrefilterShader);

		auto pPrefilterPipelineSignature = VulkanPipelineSignature::Builder(mContext, pPrefilterShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), mColorRtLoadStoreOp, EResourceBarrier::efRenderTarget, EResourceBarrier::efCopy_Source)
			.CombineAsSubpass()
			.Build();

		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"))
			.BindRenderPass(pTempVulkanRenderPass)
			.Build();

		auto* pPrefilterPipeline = VulkanPipeline::Builder(mContext.device)
			.BindRenderPass(pTempVulkanRenderPass)
			.BindSignature(pPrefilterPipelineSignature)
			.SetRasterizer(ECullMode::eNone)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.Build();

		// matrices to rotate the corresponding faces of the cube to the right place.
		eastl::array<Matrix4f, 6> directionMats = {
			// +X
			glm::toMat4(Quaternion(Vector3f(0.0f, glm::radians(-90.0f), 0.0f))) * glm::scale(Matrix4f(1.0f), { -1.0f, 1.0f, 1.0f }),
			// -X
			glm::toMat4(Quaternion(Vector3f(0.0f, glm::radians(90.0f), 0.0f))) * glm::scale(Matrix4f(1.0f), { -1.0f, 1.0f, 1.0f }),
			// +Y
			glm::toMat4(Quaternion(Vector3f(glm::radians(90.0f), 0.0f, 0.0f))) * glm::scale(Matrix4f(1.0f), { 1.0f, -1.0f, 1.0f }),
			// -Y
			glm::toMat4(Quaternion(Vector3f(glm::radians(-90.0f), 0.0f, 0.0f))) * glm::scale(Matrix4f(1.0f), { 1.0f, -1.0f, 1.0f }),
			// +Z
			glm::scale(Matrix4f(1.0f), { 1.0f, 1.0f, -1.0f }),
			// -Z
			glm::toMat4(Quaternion(Vector3f(glm::radians(180.0f), 0.0f, glm::radians(180.0f)))) * glm::scale(Matrix4f(1.0f), { 1.0f, 1.0f, -1.0f })
		};

		struct PushConstant
		{
			Matrix4f mvp;
			float    roughness;
			UInt32   numSample;
		};

		cmd.Reset();
		cmd.BeginRecord();
		{
			PushConstant pushConstant;
			pushConstant.numSample = 32; // due to the optimization, 32 is enough, no need 1024.
			for (UInt32 mip = 0; mip < numMip; ++mip)
			{
				pushConstant.roughness = (float)mip / (float)numMip; // the lower the mipmap level is, the blurrier reflection is.
				for (UInt32 face = 0; face < 6; ++face)
				{
					cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), EResourceBarrier::efCopy_Source, EResourceBarrier::efRenderTarget);

					ClearValue cv;
					cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
					cmd.BeginRenderPass(pTempFrameBuffer, &cv, 1);
					{
						cmd.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmd.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmd.BindPipelineSignatureNonDynamic(pPrefilterPipelineSignature.get());
						cmd.BindPipeline(pPrefilterPipeline);

						pushConstant.mvp = BuildPerspectiveMatrix(glm::radians(90.0f), 1.0f, 0.1f, 256.0f) * directionMats[face];
						cmd.PushConstants(pPrefilterPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &pushConstant);

						const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
						cmd.BindVertexBuffer(0, 1, *skybox.drawMesh.pVertexBuffer, &skybox.drawMesh.vBOffset);
						cmd.Draw(36, 1, 0, 0);
					}
					cmd.EndRenderPass();

					TextureCopyRegion copyRegion = {};
					copyRegion.baseArray = face;
					copyRegion.width = (texSize >> mip);
					copyRegion.height = (texSize >> mip);
					copyRegion.depth = 1;
					copyRegion.layer = 1;
					copyRegion.offset = 0;
					copyRegion.mipLevel = mip;
					cmd.CopyImage(*VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"),
						*VK_RESOURCE()->GetTexture("cubemap_prefilter"), copyRegion);
				}
			}
		}
		// now the cubemap is ready to use by shader
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_prefilter"), EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
		cmd.EndRecord();

		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&cmd, nullptr, nullptr, nullptr, pFence);

		pFence->Wait();
		mContext.pGraphicCommandPool->FreeCommandBuffer(cmd);
		Delete(pPrefilterPipeline);
		Delete(pTempFrameBuffer);
		Delete(pTempVulkanRenderPass);
		Delete(pFence);
		VK_RESOURCE()->DeleteRenderTarget("cubemap_prefilter_rt");
	}

	void RGDrawScenePBRNode::CreateColorRt()
	{
		SG_PROFILE_FUNCTION();

		TextureCreateDesc rtCI;
		rtCI.name = "HDRColor";
		rtCI.width = mContext.colorRts[0]->GetWidth();
		rtCI.height = mContext.colorRts[0]->GetHeight();
		rtCI.depth = mContext.colorRts[0]->GetDepth();
		rtCI.array = 1;
		rtCI.mipLevel = 1;
		rtCI.format = EImageFormat::eSfloat_R32G32B32A32;
		rtCI.sample = ESampleCount::eSample_1;
		rtCI.type = EImageType::e2D;
		rtCI.usage = EImageUsage::efColor | EImageUsage::efSample;
		rtCI.initLayout = EImageLayout::eUndefined;
		rtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;
		VK_RESOURCE()->CreateRenderTarget(rtCI);

		// translate color rt from undefined to shader read
		VulkanCommandBuffer pCmd;
		mContext.pGraphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("HDRColor"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.EndRecord();
		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&pCmd, nullptr, nullptr, nullptr, nullptr);
		mContext.pGraphicQueue->WaitIdle();
		mContext.pGraphicCommandPool->FreeCommandBuffer(pCmd);
	}

	void RGDrawScenePBRNode::DestroyColorRt()
	{
		SG_PROFILE_FUNCTION();

		VK_RESOURCE()->DeleteRenderTarget("HDRColor");
	}

	void RGDrawScenePBRNode::OnRenderDataRebuild()
	{
		//auto& defaultDesriptorSet = mpPipelineSignature->GetDescriptorSet(1, "__non_dynamic");
		//VulkanPipelineSignature::ShaderDataBinder(mpPipelineSignature.get(), mpShader, 1) // rebind all the textures to descriptor
		//	.AddCombindSamplerImage("texture_1k_mipmap_sampler", "embedded_texture_*0")
		//	.AddCombindSamplerImage("texture_1k_mipmap_sampler", "cerberus_metallic")
		//	.AddCombindSamplerImage("texture_1k_mipmap_sampler", "cerberus_roughness")
		//	.AddCombindSamplerImage("texture_1k_mipmap_sampler", "cerberus_ao")
		//	.AddCombindSamplerImage("texture_1k_mipmap_sampler", "embedded_texture_*2")
		//	.ReBind(defaultDesriptorSet);
	}

	void RGDrawScenePBRNode::LoadAndGenerateCubemapResource()
	{
		// load cube map texture
		TextureResourceLoader texLoader;
		Raw2DTexture texData = {};
		texLoader.LoadFromFile("cubemap_yokohama_rgba.ktx", texData, true, true);
		TextureCreateDesc texCI = {};
		texCI.name = "cubemap";
		texCI.width = texData.width;
		texCI.height = texData.height;
		texCI.depth = 1;
		texCI.array = texData.array;
		texCI.mipLevel = texData.mipLevel;
		texCI.format = EImageFormat::eUnorm_R8G8B8A8;
		texCI.sample = ESampleCount::eSample_1;
		texCI.usage = EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.pInitData = texData.pData;
		texCI.sizeInByte = texData.sizeInByte;
		texCI.pUserData = texData.pUserData;
		VK_RESOURCE()->CreateTexture(texCI);
		VK_RESOURCE()->FlushTextures();

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "cubemap_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)texCI.mipLevel;
		samplerCI.enableAnisotropy = true;
		VK_RESOURCE()->CreateSampler(samplerCI);

		GenerateBRDFLut();
		PreCalcIrradianceCubemap();
		PrefilterCubemap();
	}

#if SG_ENABLE_DEFERRED_SHADING
	void RGDrawScenePBRNode::CreateDeferredShadingRts()
	{
		// position deferred render target
		TextureCreateDesc rtCI = {};
		rtCI.name = "position_deferred_rt";
		rtCI.width = mContext.colorRts[0]->GetWidth();
		rtCI.height = mContext.colorRts[0]->GetHeight();
		rtCI.depth = mContext.colorRts[0]->GetDepth();
		rtCI.array = 1;
		rtCI.mipLevel = 1;
		rtCI.format = EImageFormat::eSfloat_R16G16B16A16;
		rtCI.sample = ESampleCount::eSample_1;
		rtCI.type = EImageType::e2D;
		rtCI.usage = EImageUsage::efColor | EImageUsage::efSample;
		rtCI.initLayout = EImageLayout::eUndefined;
		rtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;
		VK_RESOURCE()->CreateRenderTarget(rtCI);

		// normal deferred render target
		rtCI = {};
		rtCI.name = "normal_deferred_rt";
		rtCI.width = mContext.colorRts[0]->GetWidth();
		rtCI.height = mContext.colorRts[0]->GetHeight();
		rtCI.depth = mContext.colorRts[0]->GetDepth();
		rtCI.array = 1;
		rtCI.mipLevel = 1;
		rtCI.format = EImageFormat::eSfloat_R16G16B16A16;
		rtCI.sample = ESampleCount::eSample_1;
		rtCI.type = EImageType::e2D;
		rtCI.usage = EImageUsage::efColor | EImageUsage::efSample;
		rtCI.initLayout = EImageLayout::eUndefined;
		rtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;
		VK_RESOURCE()->CreateRenderTarget(rtCI);

		// albedo deferred render target
		rtCI = {};
		rtCI.name = "albedo_deferred_rt";
		rtCI.width = mContext.colorRts[0]->GetWidth();
		rtCI.height = mContext.colorRts[0]->GetHeight();
		rtCI.depth = mContext.colorRts[0]->GetDepth();
		rtCI.array = 1;
		rtCI.mipLevel = 1;
		rtCI.format = EImageFormat::eUnorm_R8G8B8A8;
		rtCI.sample = ESampleCount::eSample_1;
		rtCI.type = EImageType::e2D;
		rtCI.usage = EImageUsage::efColor | EImageUsage::efSample;
		rtCI.initLayout = EImageLayout::eUndefined;
		rtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;
		VK_RESOURCE()->CreateRenderTarget(rtCI);

		// metallic, roughness and ao deferred render target
		rtCI = {};
		rtCI.name = "mrao_deferred_rt";
		rtCI.width = mContext.colorRts[0]->GetWidth();
		rtCI.height = mContext.colorRts[0]->GetHeight();
		rtCI.depth = mContext.colorRts[0]->GetDepth();
		rtCI.array = 1;
		rtCI.mipLevel = 1;
		rtCI.format = EImageFormat::eSfloat_R16G16B16A16;
		rtCI.sample = ESampleCount::eSample_1;
		rtCI.type = EImageType::e2D;
		rtCI.usage = EImageUsage::efColor | EImageUsage::efSample;
		rtCI.initLayout = EImageLayout::eUndefined;
		rtCI.memoryFlag = EGPUMemoryFlag::efDedicated_Memory;
		VK_RESOURCE()->CreateRenderTarget(rtCI);

		// translate color rt from undefined to shader read
		VulkanCommandBuffer pCmd;
		mContext.pGraphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("position_deferred_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("normal_deferred_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("albedo_deferred_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("mrao_deferred_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.EndRecord();
		mContext.pGraphicQueue->SubmitCommands<0, 0, 0>(&pCmd, nullptr, nullptr, nullptr, nullptr);
		mContext.pGraphicQueue->WaitIdle();
		mContext.pGraphicCommandPool->FreeCommandBuffer(pCmd);
	}

	void RGDrawScenePBRNode::DestroyDeferredShadingRts()
	{
		VK_RESOURCE()->DeleteRenderTarget("position_deferred_rt");
		VK_RESOURCE()->DeleteRenderTarget("normal_deferred_rt");
		VK_RESOURCE()->DeleteRenderTarget("albedo_deferred_rt");
		VK_RESOURCE()->DeleteRenderTarget("mrao_deferred_rt");
	}
#endif

}