#include "StdAfx.h"
#include "RGShadowNode.h"

#include "System/System.h"
#include "Render/Shader/ShaderComiler.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "RendererVulkan/Resource/VulkanGeometry.h"
#include "RendererVulkan/Resource/RenderResourceRegistry.h"
#include "RendererVulkan/Resource/CommonUBO.h"

#include "Math/MathBasic.h"

namespace SG
{

#define SG_SHADOW_MAP_SIZE 2048

	RGShadowNode::RGShadowNode(VulkanContext& context)
		:mContext(context),
		mDepthRtLoadStoreOp({ ELoadOp::eClear, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		Scene* pScene = SSystem()->GetMainScene();
		pScene->TraverseMesh([&](const Mesh& mesh)
			{
				if (mesh.GetName() == "model")
					mPushConstantModel.model = mesh.GetTransform();
			}
		);
		auto& shadowUbo = GetShadowUBO();
		shadowUbo.lightSpaceVP = SSystem()->GetMainScene()->GetDirectionalLight()->GetViewProj();

		TextureCreateDesc texCI = {};
		texCI.name = "shadow map";
		texCI.width = SG_SHADOW_MAP_SIZE;
		texCI.height = SG_SHADOW_MAP_SIZE;
		texCI.depth = 1;
		texCI.array = 1;
		texCI.mipLevel = 1;

		texCI.usage = EImageUsage::efDepth_Stencil | EImageUsage::efSample;
		texCI.type = EImageType::e2D;
		texCI.sample = ESampleCount::eSample_1;
		texCI.format = EImageFormat::eUnorm_D16;
		texCI.initLayout = EImageLayout::eUndefined;
		VK_RESOURCE()->CreateRenderTarget(texCI, true);

		mpModelGeometry = VK_RESOURCE()->GetGeometry("model");

		SamplerCreateDesc samplerCI = {};
		samplerCI.name = "shadow_sampler";
		samplerCI.filterMode = EFilterMode::eLinear;
		samplerCI.mipmapMode = EFilterMode::eLinear;
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
		mpShadowPipelineSignature->UploadUniformBufferData("shadowUbo", &shadowUbo);

		ClearValue cv = {};
		cv.depthStencil.depth = 1.0f;
		cv.depthStencil.stencil = 0;
		AttachResource(0, { VK_RESOURCE()->GetRenderTarget("shadow map"), mDepthRtLoadStoreOp, cv, EResourceBarrier::efUndefined, EResourceBarrier::efDepth_Stencil_Read_Only });
	}

	RGShadowNode::~RGShadowNode()
	{
		Memory::Delete(mpShadowPipeline);
	}

	void RGShadowNode::Reset()
	{
	}

	void RGShadowNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpShadowPipeline = VulkanPipeline::Builder(mContext.device)
			.SetDepthStencil(true)
			.SetColorBlend(false)
			.SetRasterizer(VK_CULL_MODE_FRONT_BIT)
			.SetDynamicStates(VK_DYNAMIC_STATE_DEPTH_BIAS)
			.BindSignature(mpShadowPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpShadowShader.get())
			.Build();
	}

	void RGShadowNode::Update(UInt32 frameIndex)
	{
		SSystem()->GetMainScene()->TraverseMesh([&](const Mesh& mesh)
			{
				if (mesh.GetName() == "model")
				{
					mPushConstantModel.model = mesh.GetTransform();
					return;
				}
			});
	}

	void RGShadowNode::Draw(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		pBuf.SetViewport(SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE, 0.0f, 1.0f);
		pBuf.SetScissor({ 0, 0, SG_SHADOW_MAP_SIZE, SG_SHADOW_MAP_SIZE });

		pBuf.BindPipeline(mpShadowPipeline);
		pBuf.BindPipelineSignature(mpShadowPipelineSignature.get());

		pBuf.SetDepthBias(20.0f, 0.0f, 4.0f);

		UInt64 offset[1] = { 0 };
		VulkanBuffer* pVertexBuffer = mpModelGeometry->GetVertexBuffer();
		VulkanBuffer* pIndexBuffer = mpModelGeometry->GetIndexBuffer();
		pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
		pBuf.BindIndexBuffer(*pIndexBuffer, 0);

		pBuf.PushConstants(mpShadowPipelineSignature.get(), EShaderStage::efVert, sizeof(PushConstant), 0, &mPushConstantModel);
		UInt32 indexCount = pIndexBuffer->SizeInByteCPU() / sizeof(UInt32);
		pBuf.DrawIndexed(indexCount, 1, 0, 0, 1);
	}

}