#include "StdAfx.h"
#include "RGDrawSceneNode.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Render/Shader/ShaderComiler.h"
#include "Archive/ResourceLoader/RenderResourceLoader.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/Resource/RenderMesh.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Resource/CommonUBO.h"

namespace SG
{

	RGDrawSceneNode::RGDrawSceneNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
		// load scene ubos
		{
			Scene* pScene = SSystem()->GetMainScene();
			pScene->TraversePointLight([&](const PointLight& light)
				{
					mpPointLight = &light;
				});

			mpCamera = pScene->GetMainCamera();
			auto& cameraUbo = GetCameraUBO();
			cameraUbo.proj = mpCamera->GetProjMatrix();

			auto& lightUbo = GetLightUBO();
			auto* pDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
			lightUbo.lightSpaceVP = pDirectionalLight->GetViewProj();
			lightUbo.directionalColor = { pDirectionalLight->GetColor(), 1.0f };
			lightUbo.viewDirection = glm::normalize(pDirectionalLight->GetDirection());

			auto& skyboxUbo = GetSkyboxUBO();
			skyboxUbo.proj = cameraUbo.proj;

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
		mpSkyboxShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("phone", mpShader.get());
		compiler.CompileGLSLShader("skybox", mpSkyboxShader.get());

		VulkanCommandBuffer pCmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("shadow map"), EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);

		mpSkyboxPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpSkyboxShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
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

	RGDrawSceneNode::~RGDrawSceneNode()
	{
		Memory::Delete(mpSkyboxPipeline);
		Memory::Delete(mpPipeline);
	}

	void RGDrawSceneNode::Reset()
	{
		ClearResources();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv });

		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT);

		auto& skyboxUbo = GetSkyboxUBO();
		auto& cameraUbo = GetCameraUBO();
		cameraUbo.proj = mpCamera->GetProjMatrix();
		skyboxUbo.proj = cameraUbo.proj;
		cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
		VK_RESOURCE()->UpdataBufferData("cameraUbo", &cameraUbo);
	}

	void RGDrawSceneNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpSkyboxPipeline = VulkanPipeline::Builder(mContext.device)
			.SetRasterizer(VK_CULL_MODE_FRONT_BIT)
			.BindSignature(mpSkyboxPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpSkyboxShader.get())
			.Build();

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpShader.get())
			.Build();
	}

	void RGDrawSceneNode::Update(UInt32 frameIndex)
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

	void RGDrawSceneNode::Draw(RGDrawInfo& context)
	{
		auto& pBuf = *context.pCmd;

		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignature(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			const RenderMesh& skybox = VK_RESOURCE()->GetSkyboxRenderMeshData();
			pBuf.BindVertexBuffer(0, 1, *skybox.pVertexBuffer, &skybox.vBOffset);
			pBuf.Draw(36, 1, 0, 0);
		}

		// 2. Draw Scene
		DrawScene(pBuf);
	}

	void RGDrawSceneNode::DrawScene(VulkanCommandBuffer& pBuf)
	{
		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		//pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		pBuf.BindPipelineSignature(mpPipelineSignature.get());
		pBuf.BindPipeline(mpPipeline);

		VK_RESOURCE()->TraverseStaticRenderMesh([&](const RenderMesh& renderMesh)
			{
				pBuf.BindVertexBuffer(0, 1, *renderMesh.pVertexBuffer, &renderMesh.vBOffset);
				pBuf.BindIndexBuffer(*renderMesh.pIndexBuffer, renderMesh.iBOffset);

				// TODO: not to use push constant, use read write buffer.
				pBuf.PushConstants(mpPipelineSignature.get(), EShaderStage::efVert, sizeof(PerMeshRenderData), 0, &renderMesh.renderData);
				pBuf.DrawIndexed(static_cast<UInt32>(renderMesh.iBSize / sizeof(UInt32)), 1, 0, 0, 0);
			});
	}

}