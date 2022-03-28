#include "StdAfx.h"
#include "RGDrawScenePBRNode.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Render/Shader/ShaderComiler.h"
#include "Render/CommonRenderData.h"
#include "Archive/ResourceLoader/RenderResourceLoader.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

#define IRRADIANCE_CUBEMAP_TEX_SIZE 64
#define PREFILTER_CUBEMAP_TEX_SIZE  512
#define BRDF_LUT_TEX_SIZE 512

	RGDrawScenePBRNode::RGDrawScenePBRNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
		// load scene ubos
		{
			auto pScene = SSystem()->GetMainScene();
			pScene->TraversePointLight([&](const PointLight& light)
				{
					mpPointLight = &light;
				});

			mpCamera = pScene->GetMainCamera();
			auto& cameraUbo = GetCameraUBO();
			cameraUbo.proj = mpCamera->GetProjMatrix();

			auto& skyboxUbo = GetSkyboxUBO();
			skyboxUbo.proj = cameraUbo.proj;

			auto& lightUbo = GetLightUBO();
			auto* pDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
			lightUbo.lightSpaceVP = pDirectionalLight->GetViewProj();
			lightUbo.directionalColor = { pDirectionalLight->GetColor(), 1.0f };
			lightUbo.viewDirection = glm::normalize(pDirectionalLight->GetDirection());

			auto& compositionUbo = GetCompositionUBO();
			compositionUbo.gamma = 2.2f;
			compositionUbo.exposure = 1.0f;
		}

		// init render resource
#ifdef SG_ENABLE_HDR
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
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv });
	}

	RGDrawScenePBRNode::~RGDrawScenePBRNode()
	{
		Memory::Delete(mpSkyboxPipeline);
		Memory::Delete(mpInstancePipeline);
		Memory::Delete(mpPipeline);
	}

	void RGDrawScenePBRNode::Reset()
	{
		ClearResources();
#ifdef SG_ENABLE_HDR
		VK_RESOURCE()->DeleteRenderTarget("HDRColor");
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

		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("HDRColor"), mColorRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efShader_Resource });
#endif
		cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv });

		auto* window = OperatingSystem::GetMainWindow();
		const float ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT, 0.01f, 256.0f);

		auto& skyboxUbo = GetSkyboxUBO();
		auto& cameraUbo = GetCameraUBO();
		cameraUbo.proj = mpCamera->GetProjMatrix();
		skyboxUbo.proj = cameraUbo.proj;
		cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
		VK_RESOURCE()->UpdataBufferData("cameraUbo", &cameraUbo);
		VK_RESOURCE()->UpdataBufferData("skyboxUbo", &skyboxUbo);
	}

	void RGDrawScenePBRNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpSkyboxPipeline = VulkanPipeline::Builder(mContext.device)
			.SetRasterizer(VK_CULL_MODE_FRONT_BIT)
			.SetColorBlend(false)
			.BindSignature(mpSkyboxPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpSkyboxShader.get())
			.Build();

		mpInstancePipeline = VulkanPipeline::Builder(mContext.device)
			.SetInputVertexRange(sizeof(float) * 6, 0)
			.SetInputVertexRange(sizeof(float) * 4, 1)
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

	void RGDrawScenePBRNode::Update(UInt32 frameIndex)
	{
#ifndef SG_ENABLE_HDR
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp, cv });
#endif 
		if (mpCamera->IsViewDirty())
		{
			auto& skyboxUbo = GetSkyboxUBO();
			auto& cameraUbo = GetCameraUBO();
			cameraUbo.viewPos = mpCamera->GetPosition();
			cameraUbo.view = mpCamera->GetViewMatrix();
			skyboxUbo.model = Matrix4f(Matrix3f(cameraUbo.view)); // eliminate the translation part of the matrix
			cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
			VK_RESOURCE()->UpdataBufferData("cameraUbo", &cameraUbo);
			VK_RESOURCE()->UpdataBufferData("skyboxUbo", &skyboxUbo);
			mpCamera->ViewBeUpdated();
		}

		if (mpPointLight->IsDirty())
		{
			auto& lightUbo = GetLightUBO();
			lightUbo.pointLightColor = mpPointLight->GetColor();
			lightUbo.pointLightRadius = mpPointLight->GetRadius();
			lightUbo.pointLightPos = mpPointLight->GetPosition();
			mpPointLight->BeUpdated();
			VK_RESOURCE()->UpdataBufferData("lightUbo", &lightUbo);
		}

		auto& compositionUbo = GetCompositionUBO();
		VK_RESOURCE()->UpdataBufferData("compositionUbo", &compositionUbo);
	}

	void RGDrawScenePBRNode::Draw(RGDrawInfo& context)
	{
		auto& pBuf = *context.pCmd;

		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignature(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
			pBuf.BindVertexBuffer(0, 1, *skybox.pVertexBuffer, &skybox.vBOffset);
			pBuf.Draw(36, 1, 0, 0);
		}

		// 2. Draw Scene
		DrawScene(pBuf);
	}

	void RGDrawScenePBRNode::DrawScene(VulkanCommandBuffer& pBuf)
	{
		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		//pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		// 1.1 Forward Mesh Pass
		pBuf.BindPipeline(mpPipeline);
		pBuf.BindPipelineSignature(mpPipelineSignature.get());
		VK_RESOURCE()->TraverseStaticMeshDrawCall([&](const DrawCall& dc)
			{
				pBuf.BindVertexBuffer(0, 1, *dc.pVertexBuffer, &dc.vBOffset);
				pBuf.BindIndexBuffer(*dc.pIndexBuffer, dc.iBOffset);

				pBuf.DrawIndexed(static_cast<UInt32>(dc.iBSize / sizeof(UInt32)), 1, 0, 0, dc.objectId /* corresponding to gl_BaseInstance */);
			});

		// 1.2 Forward Instanced Mesh Pass
		pBuf.BindPipelineSignature(mpInstancePipelineSignature.get());
		pBuf.BindPipeline(mpInstancePipeline);
		VK_RESOURCE()->TraverseStaticMeshInstancedDrawCall([&](const DrawCall& dc)
			{
				pBuf.BindVertexBuffer(0, 1, *dc.pVertexBuffer, &dc.vBOffset);
				UInt64 offset = 0;
				pBuf.BindVertexBuffer(1, 1, *dc.pInstanceBuffer, &offset);
				pBuf.BindIndexBuffer(*dc.pIndexBuffer, dc.iBOffset);

				pBuf.DrawIndexed(static_cast<UInt32>(dc.iBSize / sizeof(UInt32)), dc.instanceCount, 0, 0, 0);
			});
	}

	void RGDrawScenePBRNode::GenerateBRDFLut()
	{
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
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetColorBlend(false)
			.SetDepthStencil(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pBrdfLutShader.get())
			.Build();

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

		mContext.graphicQueue.SubmitCommands(&cmdBuf, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(cmdBuf);

		Memory::Delete(pBrdfLutPipeline);
		Memory::Delete(pTempFrameBuffer);
		Memory::Delete(pTempVulkanRenderPass);
	}

	void RGDrawScenePBRNode::PreCalcIrradianceCubemap()
	{
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

		VulkanCommandBuffer pCmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		pCmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_irradiance"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);

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
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pIrradianceShader.get())
			.Build();

		VulkanCommandBuffer cmdBuf;
		mContext.graphicCommandPool->AllocateCommandBuffer(cmdBuf);

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

		cmdBuf.BeginRecord();
		{
			for (UInt32 mip = 0; mip < numMip; ++mip)
			{
				for (UInt32 face = 0; face < 6; ++face)
				{
					cmdBuf.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efCopy_Source, EResourceBarrier::efRenderTarget);

					cmdBuf.BeginRenderPass(pTempFrameBuffer);
					{
						cmdBuf.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmdBuf.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmdBuf.BindPipelineSignature(pIrradiancePipelineSignature.get());
						cmdBuf.BindPipeline(pIrradiancePipeline);

						PushConstant pushConstant;
						pushConstant.mvp = BuildPerspectiveMatrix(glm::radians(90.0f), 1.0f, 0.1f, 256.0f) * directionMats[face];
						pushConstant.deltaPhi = (2.0f * float(PI)) / 180.0f;
						pushConstant.deltaTheta = (0.5f * float(PI)) / 64.0f;
						cmdBuf.PushConstants(pIrradiancePipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &pushConstant);

						const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
						cmdBuf.BindVertexBuffer(0, 1, *skybox.pVertexBuffer, &skybox.vBOffset);
						cmdBuf.Draw(36, 1, 0, 0);
					}
					cmdBuf.EndRenderPass();

					TextureCopyRegion copyRegion = {};
					copyRegion.baseArray = face;
					copyRegion.width = (texSize >> mip);
					copyRegion.height = (texSize >> mip);
					copyRegion.depth = 1;
					copyRegion.layer = 1;
					copyRegion.offset = 0;
					copyRegion.mipLevel = mip;
					cmdBuf.CopyImage(*VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"),
						*VK_RESOURCE()->GetTexture("cubemap_irradiance"), copyRegion);
				}
			}
		}
		cmdBuf.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_irradiance"), EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
		cmdBuf.EndRecord();

		mContext.graphicQueue.SubmitCommands(&cmdBuf, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(cmdBuf);

		Memory::Delete(pIrradiancePipeline);
		Memory::Delete(pTempFrameBuffer);
		Memory::Delete(pTempVulkanRenderPass);
		VK_RESOURCE()->DeleteRenderTarget("cubemap_irradiance_rt");
	}

	void RGDrawScenePBRNode::PrefilterCubemap()
	{
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

		VulkanCommandBuffer pCmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Source);
		pCmd.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_prefilter"), EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);

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
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetDepthStencil(false)
			.SetColorBlend(false)
			.SetDynamicStates()
			.BindRenderPass(pTempVulkanRenderPass)
			.BindShader(pPrefilterShader.get())
			.Build();

		VulkanCommandBuffer cmdBuf;
		mContext.graphicCommandPool->AllocateCommandBuffer(cmdBuf);

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

		cmdBuf.BeginRecord();
		{
			PushConstant pushConstant;
			pushConstant.numSample = 32; // due to the optimization, 32 is enough, no need 1024.
			for (UInt32 mip = 0; mip < numMip; ++mip)
			{
				pushConstant.roughness = (float)mip / (float)numMip; // the lower the mipmap level is, the blurrier reflection is.
				for (UInt32 face = 0; face < 6; ++face)
				{
					cmdBuf.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"), EResourceBarrier::efCopy_Source, EResourceBarrier::efRenderTarget);

					cmdBuf.BeginRenderPass(pTempFrameBuffer);
					{
						cmdBuf.SetViewport((float)(texSize >> mip), (float)(texSize >> mip), 0.0f, 1.0f);
						cmdBuf.SetScissor({ 0, 0, (int)(texSize >> mip), (int)(texSize >> mip) });

						cmdBuf.BindPipelineSignature(pPrefilterPipelineSignature.get());
						cmdBuf.BindPipeline(pPrefilterPipeline);

						pushConstant.mvp = BuildPerspectiveMatrix(glm::radians(90.0f), 1.0f, 0.1f, 256.0f) * directionMats[face];
						cmdBuf.PushConstants(pPrefilterPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &pushConstant);

						const DrawCall& skybox = VK_RESOURCE()->GetSkyboxDrawCall();
						cmdBuf.BindVertexBuffer(0, 1, *skybox.pVertexBuffer, &skybox.vBOffset);
						cmdBuf.Draw(36, 1, 0, 0);
					}
					cmdBuf.EndRenderPass();

					TextureCopyRegion copyRegion = {};
					copyRegion.baseArray = face;
					copyRegion.width = (texSize >> mip);
					copyRegion.height = (texSize >> mip);
					copyRegion.depth = 1;
					copyRegion.layer = 1;
					copyRegion.offset = 0;
					copyRegion.mipLevel = mip;
					cmdBuf.CopyImage(*VK_RESOURCE()->GetRenderTarget("cubemap_prefilter_rt"),
						*VK_RESOURCE()->GetTexture("cubemap_prefilter"), copyRegion);
				}
			}
		}
		// now the cubemap is ready to use by shader
		cmdBuf.ImageBarrier(VK_RESOURCE()->GetTexture("cubemap_prefilter"), EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
		cmdBuf.EndRecord();

		mContext.graphicQueue.SubmitCommands(&cmdBuf, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(cmdBuf);

		Memory::Delete(pPrefilterPipeline);
		Memory::Delete(pTempFrameBuffer);
		Memory::Delete(pTempVulkanRenderPass);
		VK_RESOURCE()->DeleteRenderTarget("cubemap_prefilter_rt");
	}

}