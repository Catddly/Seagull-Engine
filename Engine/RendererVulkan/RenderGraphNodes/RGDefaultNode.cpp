#include "StdAfx.h"
#include "RGDefaultNode.h"

#include "System/System.h"
#include "System/Logger.h"

#include "Render/ShaderComiler.h"

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
		mUBO.proj = mpCamera->GetProjMatrix();
		//mUBO.proj = BuildPerspectiveMatrix(glm::radians(45.0f), 1.0f, 1.0, 96.0f);
		mUBO.pad = 0.0f;
		mpGeometry = VK_RESOURCE()->GetGeometry("model");

		mModelPosition = { 0.0f, 0.0f, 0.0f };
		mModelScale = 1.0f;
		mModelRotation = { 0.0f, 0.0f, 0.0f };
		mPushConstant.model = Matrix4f(1.0f);
		mPushConstant.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstant.model));

		auto* pDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
		mUBO.lightSpace = BuildPerspectiveMatrix(glm::radians(45.0f), 1.0f, 1.0, 96.0f) *
			BuildViewMatrixCenter(pDirectionalLight->GetPosition(), { 0.0f, 0.0f, 0.0f }, SG_ENGINE_UP_VEC());

		// init render resource
		mpShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		//compiler.CompileGLSLShader("basic", mBasicShader);
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
			.AddCombindSamplerImage("default", "shadow map")
			.Build();
	}

	RGDefaultNode::~RGDefaultNode()
	{
		Memory::Delete(mpPipeline);
	}

	void RGDefaultNode::Reset()
	{
		ClearResources();

		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT);
		mUBO.proj = mpCamera->GetProjMatrix();
		VK_RESOURCE()->UpdataBufferData("ubo", &mUBO);
	}

	void RGDefaultNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.BindSignature(mpPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpShader.get())
			.Build();
	}

	void RGDefaultNode::Update(float deltaTime, UInt32 frameIndex)
	{
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp });
		AttachResource(1, { mContext.depthRt, mDepthRtLoadStoreOp });

		static float totalTime = 0.0f;
		static float speed = 2.5f;
		mPushConstant.model[3][0] = 0.5f * Sin(totalTime);
		mPushConstant.inverseTransposeModel = glm::transpose(glm::inverse(mPushConstant.model));

		bool bNeedUploadData = false;
		if (mpCamera->IsViewDirty())
		{
			//Scene* pScene = SSystem()->GetMainScene();
			mUBO.viewPos = mpCamera->GetPosition();
			//mUBO.viewPos = { 0.0f, 0.0f, -4.0f };
			mUBO.view = mpCamera->GetViewMatrix();
			//mUBO.view = BuildViewMatrixCenter(mUBO.viewPos, { 0.0f, 0.0f, 0.0f }, SG_ENGINE_UP_VEC());
			bNeedUploadData = true;
		}

		if (mpPointLight->IsDirty())
		{
			mUBO.position = mpPointLight->GetPosition();
			mUBO.color = mpPointLight->GetColor();
			mUBO.radius = mpPointLight->GetRadius();
			mpPointLight->BeUpdated();
			bNeedUploadData = true;
		}

		if (bNeedUploadData)
			mpPipelineSignature->UploadUniformBufferData("ubo", &mUBO);

		totalTime += deltaTime * speed;
	}

	void RGDefaultNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		pBuf.BindPipelineSignature(mpPipelineSignature.get());
		pBuf.BindPipeline(mpPipeline);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer  = mpGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstant);
		const UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

}