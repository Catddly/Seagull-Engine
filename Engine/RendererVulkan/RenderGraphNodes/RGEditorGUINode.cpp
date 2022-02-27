#include "StdAfx.h"
#include "RGEditorGUINode.h"

#include "Memory/Memory.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "Render/ShaderComiler.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "imgui/imgui.h"

namespace SG
{

	RGEditorGUINode::RGEditorGUINode(VulkanContext& context)
		:mContext(context), mpRenderPass(nullptr), mCurrVertexCount(0), mCurrIndexCount(0),
		mColorRtLoadStoreOp({ ELoadOp::eLoad, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		// use imgui!
		mpGUIDriver = Memory::New<ImGuiDriver>();
		mpGUIDriver->OnInit();

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
		samplerCI.addressMode = EAddressMode::eRepeat;
		samplerCI.lodBias = 0.0f;
		samplerCI.minLod = -1000.0f;
		samplerCI.maxLod = 1000.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.enableAnisotropy = false;
		VK_RESOURCE()->CreateSampler(samplerCI);

		ShaderCompiler compiler;
		compiler.CompileGLSLShader("_imgui", mGUIShader);

		// upload texture data to the pipeline
		mpGUITextureSetLayout = VulkanDescriptorSetLayout::Builder(mContext.device)
			.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, 0, 1)
			.Build();
		VulkanDescriptorDataBinder(*mContext.pDefaultDescriptorPool, *mpGUITextureSetLayout)
			.BindImage(0, VK_RESOURCE()->GetSampler("_imgui_sampler"), VK_RESOURCE()->GetTexture("_imgui_font"))
			.Bind(mContext.imguiSet);
		mpGUIPipelineLayout = VulkanPipelineLayout::Builder(mContext.device)
			.AddDescriptorSetLayout(mpGUITextureSetLayout)
			.AddPushConstantRange(sizeof(float) * 2 * 2, 0, EShaderStage::efVert)
			.Build();
	}

	RGEditorGUINode::~RGEditorGUINode()
	{
		Memory::Delete(mpGUIPipeline);
		Memory::Delete(mpGUITextureSetLayout);
		Memory::Delete(mpGUIPipelineLayout);

		mpGUIDriver->OnShutdown();
		Memory::Delete(mpGUIDriver);
	}

	void RGEditorGUINode::Reset()
	{

	}

	void RGEditorGUINode::Prepare(VulkanRenderPass* pRenderpass)
	{
		// TODO: use shader reflection
		VertexLayout vertexBufferLayout = {
			{ EShaderDataType::eFloat2, "position" },
			{ EShaderDataType::eFloat2, "uv" },
			{ EShaderDataType::eUnorm4, "color" },
		};

		mpGUIPipeline = VulkanPipeline::Builder(mContext.device)
			.SetVertexLayout(vertexBufferLayout)
			.SetRasterizer(VK_CULL_MODE_NONE)
			.SetDepthStencil(false)
			.BindLayout(mpGUIPipelineLayout)
			.BindRenderPass(pRenderpass)
			.BindShader(&mGUIShader)
			.Build();
	}

	void RGEditorGUINode::Update(UInt32 frameIndex)
	{
		AttachResource(0, { mContext.colorRts[frameIndex], mColorRtLoadStoreOp });

		mpGUIDriver->OnDraw();
		ImGui::NewFrame();

		bool bShowDemoWindow = true;
		ImGui::ShowDemoWindow(&bShowDemoWindow);

		ImGui::Begin("Test");
		ImGui::Button("Button1");
		ImGui::End();

		ImGui::EndFrame();
	}

	void RGEditorGUINode::Execute(RGDrawContext& context)
	{
		auto& pBuf = *context.pCmd;

		ImGui::Render();

		ImDrawData* drawData = ImGui::GetDrawData();
		const int width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		const int height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (width <= 0 || height <= 0)
			return;

		const string vtxBufferName = "_imgui_vtx_" + eastl::to_string(context.frameIndex);
		const string idxBufferName = "_imgui_idx_" + eastl::to_string(context.frameIndex);

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

		pBuf.SetViewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f, false);

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
		pBuf.PushConstants(mpGUIPipelineLayout, EShaderStage::efVert, sizeof(float) * 2, 0, &scale);
		pBuf.PushConstants(mpGUIPipelineLayout, EShaderStage::efVert, sizeof(float) * 2, sizeof(float) * 2, &translate);

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

				// Bind DescriptorSet with font or user texture
				//VkDescriptorSet set = (VkDescriptorSet)pcmd->TextureId;
				pBuf.BindDescriptorSet(mpGUIPipelineLayout, 0, mContext.imguiSet);

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