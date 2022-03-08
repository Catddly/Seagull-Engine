#include "StdAfx.h"
#include "RGShadowNode.h"

#include "System/System.h"
#include "Render/ShaderComiler.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "RendererVulkan/Resource/VulkanGeometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "Math/MathBasic.h"

namespace SG
{

	RGShadowNode::RGShadowNode(VulkanContext& context)
		:mContext(context),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		Scene* pScene = SSystem()->GetMainScene();
		pScene->TraverseMesh([&](const Mesh& mesh)
			{
				VK_RESOURCE()->CreateGeometry(mesh.GetName(),
					mesh.GetVertices().data(), static_cast<UInt32>(mesh.GetVertices().size()),
					mesh.GetIndices().data(), static_cast<UInt32>(mesh.GetIndices().size()));
			}
		);

		mpDirectionalLight = SSystem()->GetMainScene()->GetDirectionalLight();
		mUBO.lightSpace = BuildPerspectiveMatrix(glm::radians(45.0f), 1.0f, 1.0, 96.0f) *
			BuildViewMatrixCenter(mpDirectionalLight->GetPosition(), { 0.0f, 0.0f, 0.0f }, SG_ENGINE_UP_VEC());

		mPushConstant.model = Matrix4f(1.0f);

		TextureCreateDesc texCI = {};
		texCI.name = "shadow map";
		texCI.width = 1024;
		texCI.height = 1024;
		texCI.depth = 1;
		texCI.array = 1;
		texCI.mipLevel = 1;

		texCI.usage = EImageUsage::efDepth_Stencil | EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.sample = ESampleCount::eSample_1;
		texCI.format = EImageFormat::eUnorm_D16;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateRenderTarget(texCI, true);

		mpGeometry = VK_RESOURCE()->GetGeometry("model");

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "default";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		mpShadowShader = VulkanShader::Create(mContext.device);
		ShaderCompiler compiler;
		compiler.CompileGLSLShader("shadow", mpShadowShader.get());

		mpShadowPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpShadowShader)
			.Build();
		mpShadowPipelineSignature->UploadUniformBufferData("shadowUbo", &mUBO);

		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	RGShadowNode::~RGShadowNode()
	{
		Memory::Delete(mpShadowPipeline);
	}

	void RGShadowNode::Reset()
	{
		//mUBO.view = BuildViewMatrixCenter(mpDirectionalLight->GetPosition(), { 0.0f, 0.0f, 0.0f }, SG_ENGINE_UP_VEC());
		//mUBO.proj = BuildOrthographicMatrix(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10.0f);
		//mpShadowPipelineSignature->UploadUniformBufferData("shadowUbo", &mUBO);
	}

	void RGShadowNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpShadowPipeline = VulkanPipeline::Builder(mContext.device)
			.SetDepthStencil(true)
			.BindSignature(mpShadowPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpShadowShader.get())
			.Build();
	}

	void RGShadowNode::Update(float deltaTime, UInt32 frameIndex)
	{
		static float totalTime = 0.0f;
		static float speed = 2.5f;
		mPushConstant.model[3][0] = 0.5f * Sin(totalTime);

		totalTime += deltaTime * speed;
	}

	void RGShadowNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport(1024.0f, 1024.0f, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, 1024, 1024 });

		pBuf.BindPipeline(mpShadowPipeline);
		pBuf.BindPipelineSignature(mpShadowPipelineSignature.get());

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer = mpGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpShadowPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstant);
		const UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

}