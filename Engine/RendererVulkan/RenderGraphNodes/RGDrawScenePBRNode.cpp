#include "StdAfx.h"
#include "RGDrawScenePBRNode.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Render/Shader/ShaderComiler.h"
#include "Render/CommonRenderData.h"
#include "Archive/ResourceLoader/RenderResourceLoader.h"
#include "Profile/Profile.h"

#include "Stl/Utility.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanTexture.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "RendererVulkan/Renderer/Renderer.h"
#include "RendererVulkan/Renderer/IndirectRenderer.h"

namespace SG
{

#define IRRADIANCE_CUBEMAP_TEX_SIZE 64
#define PREFILTER_CUBEMAP_TEX_SIZE  512
#define BRDF_LUT_TEX_SIZE 512

	RGDrawScenePBRNode::RGDrawScenePBRNode(VulkanContext& context, RenderGraph* pRenderGraph)
		:RenderGraphNode(pRenderGraph), mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
		SG_PROFILE_FUNCTION();

		// init render resource
#ifdef SG_ENABLE_HDR
		CreateColorRt();
#endif
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
		VK_RESOURCE()->CreateTexture(texCI, true);
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

		mpShader = VulkanShader::Create(mContext.device);
		mpInstanceShader = VulkanShader::Create(mContext.device);
		mpSkyboxShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("brdf", mpShader.get());
		compiler.CompileGLSLShader("skybox", mpSkyboxShader.get());
		compiler.CompileGLSLShader("brdf_instance", "brdf", mpInstanceShader.get());

		GenerateBRDFLut();
		PreCalcIrradianceCubemap();
		PrefilterCubemap();

		mpSkyboxPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpSkyboxShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		mpInstancePipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.AddCombindSamplerImage("brdf_lut_sampler", "brdf_lut")
			.AddCombindSamplerImage("irradiance_cubemap_sampler", "cubemap_irradiance")
			.AddCombindSamplerImage("prefilter_cubemap_sampler", "cubemap_prefilter")
			.Build();

		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.AddCombindSamplerImage("brdf_lut_sampler", "brdf_lut")
			.AddCombindSamplerImage("irradiance_cubemap_sampler", "cubemap_irradiance")
			.AddCombindSamplerImage("prefilter_cubemap_sampler", "cubemap_prefilter")
			.Build();

#ifdef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#endif
		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil });

#ifndef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp, cv });
#endif
	}

	RGDrawScenePBRNode::~RGDrawScenePBRNode()
	{
		SG_PROFILE_FUNCTION();

		Delete(mpSkyboxPipeline);
		Delete(mpInstancePipeline);
		Delete(mpPipeline);
	}

	void RGDrawScenePBRNode::Reset()
	{
		SG_PROFILE_FUNCTION();

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
	}

	void RGDrawScenePBRNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		SG_PROFILE_FUNCTION();

		mpSkyboxPipeline = VulkanPipeline::Builder(mContext.device)
			.SetRasterizer(ECullMode::eFront)
			.SetColorBlend(false)
			.BindSignature(mpSkyboxPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpSkyboxShader.get())
			.Build();

		mpInstancePipeline = VulkanPipeline::Builder(mContext.device)
			.SetInputVertexRange(sizeof(Vertex), EVertexInputRate::ePerVertex)
			.SetInputVertexRange(sizeof(PerInstanceData), EVertexInputRate::ePerInstance)
			.SetColorBlend(false)
			.BindSignature(mpInstancePipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpInstanceShader.get())
			.Build();

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.SetColorBlend(false)
			.BindSignature(mpPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpShader.get())
			.Build();
	}

	void RGDrawScenePBRNode::Draw(RGDrawInfo& context)
	{
		SG_PROFILE_FUNCTION();

		auto& pBuf = *context.pCmd;
		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efTop_Of_Pipeline, 2);

		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignature(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
			pBuf.BindVertexBuffer(0, 1, *skybox.drawMesh.pVertexBuffer, &skybox.drawMesh.vBOffset);
			pBuf.Draw(36, 1, 0, 0);
		}

		// 2. Draw Scene
		pBuf.BeginQuery(mContext.pPipelineStatisticsQueryPool, 0);
		DrawScene(pBuf);
		pBuf.EndQuery(mContext.pPipelineStatisticsQueryPool, 0);

		pBuf.WriteTimeStamp(mContext.pTimeStampQueryPool, EPipelineStage::efBottom_Of_Pipeline, 3);
	}

	void RGDrawScenePBRNode::DrawScene(VulkanCommandBuffer& pBuf)
	{
		SG_PROFILE_FUNCTION();

		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		//pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		IndirectRenderer::Begin(&pBuf);
		// 1.1 Forward Mesh Pass
		pBuf.BindPipeline(mpPipeline);
		pBuf.BindPipelineSignature(mpPipelineSignature.get());
		IndirectRenderer::Draw(EMeshPass::eForward);

		// 1.2 Forward Instanced Mesh Pass
		pBuf.BindPipeline(mpInstancePipeline);
		pBuf.BindPipelineSignature(mpInstancePipelineSignature.get());
		IndirectRenderer::Draw(EMeshPass::eForwardInstanced);
		IndirectRenderer::End();
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
		compiler.CompileGLSLShader("composition", "generate_brdf_lut", pBrdfLutShader.get());

		auto pBrdfLutPipelineSignature = VulkanPipelineSignature::Builder(mContext, pBrdfLutShader)
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("brdf_lut"), mColorRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource)
			.CombineAsSubpass()
			.Build();

		ClearValue cv;
		cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("brdf_lut"), cv)
			.BindRenderPass(pTempVulkanRenderPass)
			.Build();

		auto* pBrdfLutPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(pBrdfLutPipelineSignature.get())
			.SetRasterizer(ECullMode::eNone)
			.SetColorBlend(false)
			.SetDepthStencil(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pBrdfLutShader.get())
			.Build();

		VulkanFence* pFence = VulkanFence::Create(mContext.device);
		VulkanCommandBuffer cmdBuf;
		mContext.graphicCommandPool->AllocateCommandBuffer(cmdBuf);

		cmdBuf.BeginRecord();
		cmdBuf.BeginRenderPass(pTempFrameBuffer);
		{
			cmdBuf.SetViewport((float)texSize, (float)texSize, 0.0f, 1.0f);
			cmdBuf.SetScissor({ 0, 0, (int)texSize, (int)texSize });

			cmdBuf.BindPipeline(pBrdfLutPipeline);
			// draw screen triangle
			cmdBuf.Draw(3, 1, 0, 0);
		}
		cmdBuf.EndRenderPass();
		cmdBuf.EndRecord();

		mContext.graphicQueue.SubmitCommands(&cmdBuf, nullptr, nullptr, pFence);

		pFence->Wait();
		mContext.graphicCommandPool->FreeCommandBuffer(cmdBuf);
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
		VK_RESOURCE()->CreateTexture(texCI, true);

		// create for drawing and copying
		texCI.name = "cubemap_irradiance_rt";
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.usage = EImageUsage::efColor | EImageUsage::efTransfer_Src;
		VK_RESOURCE()->CreateRenderTarget(texCI);

		VulkanFence* pFence = VulkanFence::Create(mContext.device);

		VulkanCommandBuffer cmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(cmd);
		cmd.BeginRecord();
		cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_irradiance"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		cmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&cmd, nullptr, nullptr, pFence);
		pFence->WaitAndReset();

		auto pIrradianceShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("irradiance_cubemap", pIrradianceShader.get());

		auto pIrradiancePipelineSignature = VulkanPipelineSignature::Builder(mContext, pIrradianceShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), mColorRtLoadStoreOp, EResourceBarrier::efRenderTarget, EResourceBarrier::efCopy_Source)
			.CombineAsSubpass()
			.Build();

		ClearValue cv;
		cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), cv)
			.BindRenderPass(pTempVulkanRenderPass)
			.Build();

		auto* pIrradiancePipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(pIrradiancePipelineSignature.get())
			.SetRasterizer(ECullMode::eNone)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pIrradianceShader.get())
			.Build();

		// matrixs to rotate the corresponding faces of the cube to the right place.
		eastl::array<Matrix4f, 6> directionMats = {
			// +X
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), glm::radians(90.0f), 0.0f))),
			// -X
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), glm::radians(-90.0f), 0.0f))),
			// +Y
			glm::toMat4(Quternion(Vector3f(glm::radians(-90.0f), 0.0f, 0.0f))),
			// -Y
			glm::toMat4(Quternion(Vector3f(glm::radians(90.0f), 0.0f, 0.0f))),
			// +Z
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), 0.0f, 0.0f))),
			// -Z
			glm::toMat4(Quternion(Vector3f(0.0f, 0.0f, glm::radians(180.0f))))
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

					cmd.BeginRenderPass(pTempFrameBuffer);
					{
						cmd.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmd.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmd.BindPipelineSignature(pIrradiancePipelineSignature.get());
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

		mContext.graphicQueue.SubmitCommands(&cmd, nullptr, nullptr, pFence);

		pFence->Wait();

		mContext.graphicCommandPool->FreeCommandBuffer(cmd);
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
		VK_RESOURCE()->CreateTexture(texCI, true);

		// create for drawing and copying
		texCI.name = "cubemap_prefilter_rt";
		texCI.array = 1;
		texCI.mipLevel = 1;
		texCI.usage = EImageUsage::efColor | EImageUsage::efTransfer_Src;
		VK_RESOURCE()->CreateRenderTarget(texCI);

		VulkanFence* pFence = VulkanFence::Create(mContext.device);

		VulkanCommandBuffer cmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(cmd);
		cmd.BeginRecord();
		cmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		cmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_prefilter"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		cmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&cmd, nullptr, nullptr, pFence);
		pFence->WaitAndReset();

		auto pPrefilterShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("prefilter_cubemap", pPrefilterShader.get());

		auto pPrefilterPipelineSignature = VulkanPipelineSignature::Builder(mContext, pPrefilterShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		// do once irradiance command
		auto* pTempVulkanRenderPass = VulkanRenderPass::Builder(mContext.device)
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), mColorRtLoadStoreOp, EResourceBarrier::efRenderTarget, EResourceBarrier::efCopy_Source)
			.CombineAsSubpass()
			.Build();

		ClearValue cv;
		cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		auto* pTempFrameBuffer = VulkanFrameBuffer::Builder(mContext.device)
			.AddRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), cv)
			.BindRenderPass(pTempVulkanRenderPass)
			.Build();

		auto* pPrefilterPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(pPrefilterPipelineSignature.get())
			.SetRasterizer(ECullMode::eNone)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pPrefilterShader.get())
			.Build();

		// matrixs to rotate the corresponding faces of the cube to the right place.
		eastl::array<Matrix4f, 6> directionMats = {
			// +X
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), glm::radians(90.0f), 0.0f))),
			// -X
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), glm::radians(-90.0f), 0.0f))),
			// +Y
			glm::toMat4(Quternion(Vector3f(glm::radians(-90.0f), 0.0f, 0.0f))),
			// -Y
			glm::toMat4(Quternion(Vector3f(glm::radians(90.0f), 0.0f, 0.0f))),
			// +Z
			glm::toMat4(Quternion(Vector3f(glm::radians(180.0f), 0.0f, 0.0f))),
			// -Z
			glm::toMat4(Quternion(Vector3f(0.0f, 0.0f, glm::radians(180.0f))))
		};

		struct PushConstant
		{
			Matrix4f mvp;
			float    roughness;
			UINT32   numSample;
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

					cmd.BeginRenderPass(pTempFrameBuffer);
					{
						cmd.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmd.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmd.BindPipelineSignature(pPrefilterPipelineSignature.get());
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

		mContext.graphicQueue.SubmitCommands(&cmd, nullptr, nullptr, pFence);

		pFence->Wait();
		mContext.graphicCommandPool->FreeCommandBuffer(cmd);
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
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("HDRColor"), EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);
	}

	void RGDrawScenePBRNode::DestroyColorRt()
	{
		SG_PROFILE_FUNCTION();

		VK_RESOURCE()->DeleteRenderTarget("HDRColor");
	}

}