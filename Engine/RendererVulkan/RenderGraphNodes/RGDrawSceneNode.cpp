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

#include "RendererVulkan/Resource/VulkanGeometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Resource/CommonUBO.h"

namespace SG
{

#define IRRADIANCE_CUBEMAP_PREFILTER_TEX_SIZE 64

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

			mpModelGeometry = VK_RESOURCE()->GetGeometry("model");
			mpGridGeometry = VK_RESOURCE()->GetGeometry("grid");

			mPushConstantGeo.model = pScene->GetMesh("model")->GetTransform();
			mPushConstantGeo.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstantGeo.model));

			mPushConstantGrid.model = pScene->GetMesh("grid")->GetTransform();
			mPushConstantGrid.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstantGrid.model));

			auto& lightUbo = GetLightUBO();
			auto* pDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
			lightUbo.lightSpaceVP = pDirectionalLight->GetViewProj();
			lightUbo.directionalColor = { pDirectionalLight->GetColor(), 1.0f };
			lightUbo.viewDirection = glm::normalize(pDirectionalLight->GetDirection());

			auto& skyboxUbo = GetSkyboxUBO();
			skyboxUbo.proj = cameraUbo.proj;

			// skybox
			mpSkyboxGeometry = VK_RESOURCE()->GetGeometry("skybox");

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

		PreCalcIrradianceCubemap();

		mpSkyboxPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpSkyboxShader)
			.AddCombindSamplerImage("cubemap_sampler", "cubemap")
			.Build();

		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.AddCombindSamplerImage("irradiance_cubemap_sampler", "cubemap_irradiance")
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

		SSystem()->GetMainScene()->TraverseMesh([&](const Mesh& mesh)
			{
				if (mesh.GetName() == "model")
				{
					mPushConstantGeo.model = mesh.GetTransform();
					mPushConstantGeo.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstantGeo.model));
					return;
				}
			});
	}

	void RGDrawSceneNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		// 1. Draw Skybox
		{
			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 1.0f, 1.0f); // set z to 1.0
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipelineSignature(mpSkyboxPipelineSignature.get());
			pBuf.BindPipeline(mpSkyboxPipeline);

			UInt64 offset[1] = { 0 };
			VulkanBuffer* pVertexBuffer = mpSkyboxGeometry->GetVertexBuffer();
			pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);

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

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpModelGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer = mpModelGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstantGeo);
		UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);

		pVertexBuffer = mpGridGeometry->GetVertexBuffer();
		pIndexBuffer = mpGridGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstantGrid);
		indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

	void RGDrawSceneNode::PreCalcIrradianceCubemap()
	{
		const UInt32 texSize = IRRADIANCE_CUBEMAP_PREFILTER_TEX_SIZE;
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
			.BindRenderTarget(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), mColorRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efRenderTarget)
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

						UInt64 offset[1] = { 0 };
						VulkanBuffer* pVertexBuffer = mpSkyboxGeometry->GetVertexBuffer();
						cmdBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
						cmdBuf.Draw(36, 1, 0, 0);
					}
					cmdBuf.EndRenderPass();

					cmdBuf.ImageBarrier(VK_RESOURCE()->GetRenderTarget("cubemap_irradiance_rt"), EResourceBarrier::efRenderTarget, EResourceBarrier::efCopy_Source);

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

}