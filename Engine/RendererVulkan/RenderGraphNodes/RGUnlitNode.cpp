#include "StdAfx.h"
#include "RGUnlitNode.h"

#include "System/Logger.h"

#include "Render/ShaderComiler.h"
#include "Math/Transform.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "RendererVulkan/Resource/Geometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

namespace SG
{

	RGUnlitNode::RGUnlitNode(VulkanContext& context)
		: mContext(context), mpPipeline(nullptr), mpPipelineLayout(nullptr),
		// Set to default clear ops
		mColorRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care }),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eDont_Care, ELoadOp::eClear, EStoreOp::eDont_Care })
	{
		// init render resource
		ShaderCompiler compiler;
		//compiler.CompileGLSLShader("basic", mBasicShader);
		compiler.CompileGLSLShader("basic1", "phone", mBasicShader);

		BufferCreateDesc BufferCI = {};
		BufferCI.name = "CameraUniform";
		BufferCI.totalSizeInByte = sizeof(UBO);
		BufferCI.type = EBufferType::efUniform;
		VK_RESOURCE()->CreateBuffer(BufferCI);
		VK_RESOURCE()->FlushBuffers();

		mpUBOSetLayout = VulkanDescriptorSetLayout::Builder(mContext.device)
			.AddBinding(EDescriptorType::eUniform_Buffer, EShaderStage::efVert, 0, 1)
			//.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, 1, 1)
			.Build();
		VulkanDescriptorDataBinder(*mContext.pDefaultDescriptorPool, *mpUBOSetLayout)
			.BindBuffer(0, VK_RESOURCE()->GetBuffer("CameraUniform"))
			//.BindImage(1, VK_RESOURCE()->GetSampler("default"), VK_RESOURCE()->GetTexture("logo"))
			.Bind(mContext.cameraUBOSet);
		mpPipelineLayout = VulkanPipelineLayout::Builder(mContext.device)
			.AddDescriptorSetLayout(mpUBOSetLayout)
			.AddPushConstantRange(sizeof(PushConstant), 0, EShaderStage::efVert)
			.Build();
	}

	RGUnlitNode::~RGUnlitNode()
	{
		Memory::Delete(mpUBOSetLayout);
		Memory::Delete(mpPipelineLayout);

		Memory::Delete(mpPipeline);
	}

	void RGUnlitNode::BindGeometry(const char* name)
	{
		mpGeometry = VK_RESOURCE()->GetGeometry(name);
		if (!mpGeometry)
		{
			SG_LOG_ERROR("Failed to bind geometry! Geometry does not exist!");
			SG_ASSERT(false);
		}
	}

	void RGUnlitNode::SetCamera(ICamera* pCamera)
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

	void RGUnlitNode::Reset()
	{
		ClearResources();

		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		mpCamera->SetPerspective(45.0f, ASPECT);
		mCameraUBO.proj = mpCamera->GetProjMatrix();
		VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);
	}

	void RGUnlitNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		// TODO: use shader reflection
		VertexLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat3, "position" },
			//{ EShaderDataType::eFloat3, "color" },
			{ EShaderDataType::eFloat3, "normal" },
			//{ EShaderDataType::eFloat2, "uv" },
		};

		mpPipeline = VulkanPipeline::Builder(mContext.device)
			.SetVertexLayout(vertexBufferLayout)
			.BindLayout(mpPipelineLayout)
			.BindRenderPass(pRenderpass)
			.BindShader(&mBasicShader)
			.Build();
	}

	void RGUnlitNode::Update(float deltaTime, UInt32 frameIndex)
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
			VK_RESOURCE()->UpdataBufferData("CameraUniform", &mCameraUBO);
		}

		totalTime += deltaTime * speed;
	}

	void RGUnlitNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

		pBuf.BindDescriptorSet(mpPipelineLayout, 0, mContext.cameraUBOSet);
		pBuf.BindPipeline(mpPipeline);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer  = mpGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpPipelineLayout, EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstant);
		UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

}