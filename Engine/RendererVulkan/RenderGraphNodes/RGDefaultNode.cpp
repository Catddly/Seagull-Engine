#include "StdAfx.h"
#include "RGDefaultNode.h"

#include "System/Logger.h"

#include "Render/ShaderComiler.h"
#include "Math/Transform.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/Resource/Geometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	RGDefaultNode::RGDefaultNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
		// init render resource
		mpShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		//compiler.CompileGLSLShader("basic", mBasicShader);
		compiler.CompileGLSLShader("basic1", "phone", mpShader.get());

		mpPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShader)
			.Build();
	}

	RGDefaultNode::~RGDefaultNode()
	{
		Memory::Delete(mpPipeline);
	}

	void RGDefaultNode::BindGeometry(const char* name)
	{
		mpGeometry = VK_RESOURCE()->GetGeometry(name);
		if (!mpGeometry)
		{
			SG_LOG_ERROR("Failed to bind geometry! Geometry does not exist!");
			SG_ASSERT(false);
		}
	}

	void RGDefaultNode::SetCamera(ICamera* pCamera)
	{
		if (!pCamera)
		{
			SG_LOG_ERROR("Invalid camera!");
			return;
		}

		mpCamera = pCamera;

		// init buffer data
		mCameraUBO.proj = mpCamera->GetProjMatrix();
		mCameraUBO.viewPos = mpCamera->GetPosition();
		mCameraUBO.pad = 0.0f;

		mModelPosition = { 0.0f, 0.0f, 0.0f };
		mModelScale = 1.0f;
		mModelRotation = { 0.0f, 0.0f, 0.0f };
		mPushConstant.model = BuildTransformMatrix(mModelPosition, mModelScale, mModelRotation);
		mPushConstant.inverseTransposeModel = mPushConstant.model.inverse().transpose();
	}

	void RGDefaultNode::Reset()
	{
		ClearResources();

		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT);
		mCameraUBO.proj = mpCamera->GetProjMatrix();
		VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);
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
		mModelPosition(0) = 0.5f * Sin(totalTime);
		TranslateToX(mPushConstant.model, mModelPosition(0));
		mPushConstant.inverseTransposeModel = mPushConstant.model.inverse().transpose();

		if (mpCamera->IsViewDirty())
		{
			mCameraUBO.viewPos = mpCamera->GetPosition();
			mCameraUBO.view = mpCamera->GetViewMatrix();
			mpPipelineSignature->UploadUniformBufferData("cameraUbo", &mCameraUBO);
		}

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