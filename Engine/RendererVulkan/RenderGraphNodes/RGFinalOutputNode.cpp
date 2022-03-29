#include "StdAfx.h"
#include "RGFinalOutputNode.h"

#include "Memory/Memory.h"
#include "Render/CommonRenderData.h"
#include "Render/Shader/ShaderComiler.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanShader.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "imgui/imgui.h"

namespace SG
{

	RGFinalOutputNode::RGFinalOutputNode(VulkanContext& context)
		:mContext(context), mCurrVertexCount(0), mCurrIndexCount(0),
		mColorRtLoadStoreOp({ ELoadOp::eDont_Care, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		// bottom color render target present
		{
			mpCompShader = VulkanShader::Create(mContext.device);
			ShaderCompiler compiler;
			compiler.CompileGLSLShader("composition", mpCompShader.get());

			SamplerCreateDesc samplerCI = {};
			samplerCI.name = "comp_sampler";
			samplerCI.filterMode = EFilterMode::eNearest;
			samplerCI.mipmapMode = EFilterMode::eLinear;
			samplerCI.addressMode = EAddressMode::eClamp_To_Edge;
			samplerCI.lodBias = 0.0f;
			samplerCI.minLod = 0.0f;
			samplerCI.maxLod = 1.0f;
			samplerCI.maxAnisotropy = 1.0f;
			samplerCI.enableAnisotropy = false;
			VK_RESOURCE()->CreateSampler(samplerCI);

			mpCompPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpCompShader)
				.AddCombindSamplerImage("comp_sampler", "HDRColor")
				.Build();
		}

		// draw ui layer
		{
			// create imgui font texture
			ImGuiIO& io = ImGui::GetIO();

			unsigned char* pixels = nullptr;
			int width, height, bytePerPixel;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytePerPixel);
			UInt32 fontUploadSize = width * height * bytePerPixel;

			TextureCreateDesc textureCI = {};
			textureCI.name = "_imgui_font";
			textureCI.width = static_cast<UInt32>(width);
			textureCI.height = static_cast<UInt32>(height);
			textureCI.depth = 1;
			textureCI.array = 1;
			textureCI.mipLevel = 1;
			textureCI.sizeInByte = fontUploadSize;
			textureCI.pInitData = pixels;

			textureCI.format = EImageFormat::eUnorm_R8G8B8A8;
			textureCI.sample = ESampleCount::eSample_1;
			textureCI.usage = EImageUsage::efSample;
			textureCI.type = EImageType::e2D;
			if (!VK_RESOURCE()->CreateTexture(textureCI, true))
			{
				SG_LOG_ERROR("Failed to create texture!");
				SG_ASSERT(false);
			}

			// here we copy the buffer(pixels) to the image
			VK_RESOURCE()->FlushTextures();

			SamplerCreateDesc samplerCI = {};
			samplerCI.name = "_imgui_sampler";
			samplerCI.filterMode = EFilterMode::eLinear;
			samplerCI.mipmapMode = EFilterMode::eLinear;
			samplerCI.addressMode = EAddressMode::eRepeat;
			samplerCI.lodBias = 0.0f;
			samplerCI.minLod = -1000.0f;
			samplerCI.maxLod = 1000.0f;
			samplerCI.maxAnisotropy = 1.0f;
			samplerCI.enableAnisotropy = false;
			VK_RESOURCE()->CreateSampler(samplerCI);

			mpGUIShader = VulkanShader::Create(mContext.device);
			ShaderCompiler compiler;
			compiler.CompileGLSLShader("imgui/_imgui", mpGUIShader.get());

			mpGUIPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpGUIShader)
				.AddCombindSamplerImage("_imgui_sampler", "_imgui_font")
				.Build();
		}

		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { mContext.colorRts.data(), static_cast<UInt32>(mContext.colorRts.size()), mColorRtLoadStoreOp, cv,
			EResourceBarrier::efUndefined, EResourceBarrier::efPresent });
	}

	RGFinalOutputNode::~RGFinalOutputNode()
	{
		Memory::Delete(mpCompPipeline);
		Memory::Delete(mpGUIPipeline);
	}

	void RGFinalOutputNode::Reset()
	{
		mpCompPipelineSignature = VulkanPipelineSignature::Builder(mContext, mpCompShader)
			.AddCombindSamplerImage("comp_sampler", "HDRColor")
			.Build();

		ClearValue cv = {};
		cv.color = { 0.04f, 0.04f, 0.04f, 1.0f };
		AttachResource(0, { mContext.colorRts.data(), static_cast<UInt32>(mContext.colorRts.size()), mColorRtLoadStoreOp, cv,
			EResourceBarrier::efUndefined, EResourceBarrier::efPresent });
	}

	void RGFinalOutputNode::Prepare(VulkanRenderPass* pRenderpass)
	{
		mpCompPipeline = VulkanPipeline::Builder(mContext.device)
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetDynamicStates()
			.SetDepthStencil(false)
			.BindSignature(mpCompPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpCompShader.get())
			.Build();

		mpGUIPipeline = VulkanPipeline::Builder(mContext.device)
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetDynamicStates()
			.SetDepthStencil(false)
			.BindSignature(mpGUIPipelineSignature.get())
			.BindRenderPass(pRenderpass)
			.BindShader(mpGUIShader.get())
			.Build();
	}

	void RGFinalOutputNode::Draw(RGDrawInfo& context)
	{
		// 1. Draw the color rt (just a simple quad on the screen)
		{
			auto& pBuf = *context.pCmd;

			pBuf.SetViewport((float)mContext.colorRts[0]->GetWidth(), (float)mContext.colorRts[0]->GetHeight(), 0.0f, 1.0f);
			pBuf.SetScissor({ 0, 0, (int)mContext.colorRts[0]->GetWidth(), (int)mContext.colorRts[0]->GetHeight() });

			pBuf.BindPipeline(mpCompPipeline);
			pBuf.BindPipelineSignature(mpCompPipelineSignature.get());

			pBuf.Draw(3, 1, 0, 0);
		}

		// 2. Draw GUI on top of the color rt
		GUIDraw(*context.pCmd, context.frameIndex);
	}

	void RGFinalOutputNode::GUIDraw(VulkanCommandBuffer& pBuf, UInt32 frameIndex)
	{
		ImGui::Render();

		ImDrawData* drawData = ImGui::GetDrawData();
		const int width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		const int height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (width <= 0 || height <= 0)
			return;

		const string vtxBufferName = "_imgui_vtx_" + eastl::to_string(frameIndex);
		const string idxBufferName = "_imgui_idx_" + eastl::to_string(frameIndex);

		VulkanBuffer* pVertexBuffer = VK_RESOURCE()->GetBuffer(vtxBufferName);
		VulkanBuffer* pIndexBuffer = VK_RESOURCE()->GetBuffer(idxBufferName);

		// create buffer to hold the vtx and idx data
		if (drawData->TotalVtxCount > 0)
		{
			bool bNeedToCreateVtxBuffer = false;
			bool bNeedToCreateIdxBuffer = false;
			const UInt32 vtxBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
			const UInt32 idxBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

			if (pVertexBuffer && vtxBufferSize > pVertexBuffer->SizeInByteCPU()) // need to create a new one to hold the vtx buffer
			{
				VK_RESOURCE()->DeleteBuffer(vtxBufferName);
				bNeedToCreateVtxBuffer = true;
			}

			if (pIndexBuffer && idxBufferSize > pIndexBuffer->SizeInByteCPU()) // need to create a new one to hold the idx buffer
			{
				VK_RESOURCE()->DeleteBuffer(idxBufferName);
				bNeedToCreateIdxBuffer = true;
			}

			BufferCreateDesc bufferCI = {};
			if (!pVertexBuffer || bNeedToCreateVtxBuffer)
			{
				bufferCI.name = vtxBufferName.c_str();
				bufferCI.totalSizeInByte = vtxBufferSize;
				bufferCI.type = EBufferType::efVertex;
				VK_RESOURCE()->CreateBuffer(bufferCI);
			}

			if (!pIndexBuffer || bNeedToCreateIdxBuffer)
			{
				bufferCI.name = idxBufferName.c_str();
				bufferCI.totalSizeInByte = idxBufferSize;
				bufferCI.type = EBufferType::efIndex;
				VK_RESOURCE()->CreateBuffer(bufferCI);
			}

			pVertexBuffer = VK_RESOURCE()->GetBuffer(vtxBufferName);
			pIndexBuffer = VK_RESOURCE()->GetBuffer(idxBufferName);
			SG_ASSERT(pVertexBuffer && pIndexBuffer);

			UInt32 vtxOffest = 0;
			UInt32 idxOffest = 0;
			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const ImDrawList* pCmdList = drawData->CmdLists[n];
				const UInt32 vtxBufferSize = pCmdList->VtxBuffer.Size * sizeof(ImDrawVert);
				const UInt32 itxBufferSize = pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

				pVertexBuffer->UploadData<ImDrawVert>(pCmdList->VtxBuffer.Data, vtxBufferSize, vtxOffest);
				pIndexBuffer->UploadData<ImDrawIdx>(pCmdList->IdxBuffer.Data, itxBufferSize, idxOffest);

				vtxOffest += pCmdList->VtxBuffer.Size;
				idxOffest += pCmdList->IdxBuffer.Size;
			}
		}

		// Do Drawing
		if (pVertexBuffer && pIndexBuffer)
		{
			pBuf.SetViewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);

			pBuf.BindPipeline(mpGUIPipeline);

			UInt64 offset[1] = { 0 };
			pBuf.BindVertexBuffer(0, 1, *pVertexBuffer, offset);
			pBuf.BindIndexBuffer(*pIndexBuffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

			float scale[2];
			scale[0] = 2.0f / drawData->DisplaySize.x;
			scale[1] = 2.0f / drawData->DisplaySize.y;
			float translate[2];
			translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
			translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];
			pBuf.PushConstants(mpGUIPipelineSignature.get(), EShaderStage::efVert, sizeof(float) * 2, 0, &scale);
			pBuf.PushConstants(mpGUIPipelineSignature.get(), EShaderStage::efVert, sizeof(float) * 2, sizeof(float) * 2, &translate);

			ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
			ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

			// Render command lists
			// (Because we merged all buffers into a single one, we maintain our own offset into them)
			UInt32 vtxOffest = 0;
			UInt32 idxOffest = 0;
			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const ImDrawList* pCmdList = drawData->CmdLists[n];
				for (int i = 0; i < pCmdList->CmdBuffer.Size; i++)
				{
					const ImDrawCmd* pcmd = &pCmdList->CmdBuffer[i];
					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clipMin((pcmd->ClipRect.x - clipOff.x) * clipScale.x, (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
					ImVec2 clipMax((pcmd->ClipRect.z - clipOff.x) * clipScale.x, (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

					// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
					if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
					if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
					if (clipMax.x > width) { clipMax.x = static_cast<float>(width); }
					if (clipMax.y > height) { clipMax.y = static_cast<float>(height); }
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
						continue;

					// Apply scissor/clipping rectangle
					Rect scissor;
					scissor.left = (Int32)(clipMin.x);
					scissor.top = (Int32)(clipMin.y);
					scissor.right = (Int32)(clipMax.x);
					scissor.bottom = (Int32)(clipMax.y);
					pBuf.SetScissor(scissor);

					pBuf.BindPipelineSignature(mpGUIPipelineSignature.get());

					pBuf.DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + idxOffest, pcmd->VtxOffset + vtxOffest, 0);
				}
				idxOffest += pCmdList->IdxBuffer.Size;
				vtxOffest += pCmdList->VtxBuffer.Size;
			}

			// Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
			// Our last values will leak into user/application rendering IF:
			// - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
			// - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitely set that state.
			// If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
			// In theory we should aim to backup/restore those values but I am not sure this is possible.
			// We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
			Rect scissor;
			scissor.left = 0;
			scissor.top = 0;
			scissor.right = (Int32)(width);
			scissor.bottom = (Int32)(height);
			pBuf.SetScissor(scissor);
		}
	}

}