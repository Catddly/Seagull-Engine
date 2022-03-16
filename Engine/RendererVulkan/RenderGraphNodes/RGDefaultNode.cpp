#include "StdAfx.h"
#include "RGDefaultNode.h"

#include "System/System.h"
#include "System/Logger.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "Render/Shader/ShaderComiler.h"

#include "RendererVulkan/Resource/VulkanGeometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Resource/CommonUBO.h"

namespace SG
{

	RGDefaultNode::RGDefaultNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
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

		pScene->TraverseMesh([&](const Mesh& mesh)
			{
				if (mesh.GetName() == "model")
				{
					mPushConstantGeo.model = mesh.GetTransform();
					mPushConstantGeo.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstantGeo.model));
				}
				else if (mesh.GetName() == "grid")
				{
					mPushConstantGrid.model = mesh.GetTransform();
					mPushConstantGrid.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstantGrid.model));
				}
			}
		);

		auto& lightUbo = GetLightUBO();
		auto* pDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
		lightUbo.lightSpaceVP = pDirectionalLight->GetViewProj();
		lightUbo.directionalColor = { pDirectionalLight->GetColor(), 1.0f };
		lightUbo.viewDirection = glm::normalize(pDirectionalLight->GetDirection());
		lightUbo.gamma = 2.2f;

		// init render resource
		mpShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		//compiler.CompileGLSLShader("scene", mpShader.get());
		compiler.CompileGLSLShader("basic1", "phone", mpShader.get());

		VulkanCommandBuffer pCmd;
		mContext.graphicCommandPool->AllocateCommandBuffer(pCmd);
		pCmd.BeginRecord();
		pCmd.ImageBarrier(VK_RESOURCE()->GetRenderTarget("shadow map"), EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only);
		pCmd.EndRecord();
		mContext.graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.graphicQueue.WaitIdle();
		mContext.graphicCommandPool->FreeCommandBuffer(pCmd);

		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.AddCombindSamplerImage("shadow_sampler", "shadow map")
			.Build();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv });
	}

	RGDefaultNode::~RGDefaultNode()
	{
		Memory::Delete(mpPipeline);
	}

	void RGDefaultNode::Reset()
	{
		ClearResources();

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp, cv });

		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT);

		auto& cameraUbo = GetCameraUBO();
		cameraUbo.proj = mpCamera->GetProjMatrix();
		VK_RESOURCE()->UpdataBufferData("cameraUbo", &cameraUbo);
	}

	void RGDefaultNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpPipelineSignature.get())
			.SetDynamicStates()
			.BindRenderPass(pRenderpass)
			.BindShader(mpShader.get())
			.Build();
	}

	void RGDefaultNode::Update(UInt32 frameIndex)
	{
		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp, cv });

		if (mpCamera->IsViewDirty())
		{
			auto& cameraUbo = GetCameraUBO();
			cameraUbo.viewPos = mpCamera->GetPosition();
			cameraUbo.view = mpCamera->GetViewMatrix();
			cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
			mpPipelineSignature->UploadUniformBufferData("cameraUbo", &cameraUbo);
		}

		//if (mpPointLight->IsDirty())
		//{
		auto& lightUbo = GetLightUBO();
		lightUbo.pointLightColor = mpPointLight->GetColor();
		lightUbo.pointLightRadius = mpPointLight->GetRadius();
		lightUbo.pointLightPos = mpPointLight->GetPosition();
		mpPointLight->BeUpdated();
		mpPipelineSignature->UploadUniformBufferData("lightUbo", &lightUbo);
		//}

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

	void RGDefaultNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		pBuf.BindPipelineSignature(mpPipelineSignature.get());
		pBuf.BindPipeline(mpPipeline);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpModelGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer  = mpModelGeometry->GetIndexBuffer();
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

}