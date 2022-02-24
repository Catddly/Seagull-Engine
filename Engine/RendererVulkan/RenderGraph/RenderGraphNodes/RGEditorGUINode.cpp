#include "StdAfx.h"
#include "RGEditorGUINode.h"

#include "Memory/Memory.h"

#include "RendererVulkan/GUI/ImGuiDriver.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"
#include "RendererVulkan/Backend/VulkanDescriptor.h"
#include "RendererVulkan/Backend/VulkanFrameBuffer.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "imgui/imgui.h"

namespace SG
{

	RGEditorGUINode::RGEditorGUINode(VulkanContext& context)
		:mContext(context), mpRenderPass(nullptr),
		mColorRtLoadStoreOp({ ELoadOp::eLoad, EStoreOp::eStore, ELoadOp::eDont_Care, EStoreOp::eDont_Care })
	{
		// use imgui!
		mpGUIDriver = Memory::New<ImGuiDriver>();
		mpGUIDriver->OnInit();

		// create imgui font texture
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		UInt32 fontUploadSize = width * height * 4 * sizeof(char);

		TextureCreateDesc textureCI = {};
		textureCI.name = "__imgui_font";
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
		if (VK_RESOURCE()->CreateTexture(textureCI, true))
		{
			SG_LOG_ERROR("Failed to create texture!");
			SG_ASSERT(false);
		}

		// here we copy the buffer(pixels) to the image
		VK_RESOURCE()->FlushTextures();

		// upload texture data to the pipeline
		mpGUITextureSetLayout = VulkanDescriptorSetLayout::Builder(mContext.device)
			.AddBinding(EDescriptorType::eCombine_Image_Sampler, EShaderStage::efFrag, 0, 1)
			.Build();
		VulkanDescriptorDataBinder(*mContext.pDefaultDescriptorPool, *mpGUITextureSetLayout)
			.BindImage(0, VK_RESOURCE()->GetSampler("default"), VK_RESOURCE()->GetTexture("__imgui_font"))
			.Bind(mContext.imguiSet);
		mpGUIPipelineLayout = VulkanPipelineLayout::Builder(mContext.device)
			.AddDescriptorSetLayout(mpGUITextureSetLayout)
			.AddPushConstantRange(sizeof(float) * 2 * 2, 0, EShaderStage::efVert)
			.Build();

		// store the identifier
		io.Fonts->SetTexID((ImTextureID)mContext.imguiSet);
	}

	RGEditorGUINode::~RGEditorGUINode()
	{
		mpGUIDriver->OnShutdown();
		Memory::Delete(mpGUIDriver);
	}

	void RGEditorGUINode::Reset()
	{

	}

	void RGEditorGUINode::Prepare(VulkanRenderPass* pRenderpass)
	{

	}

	void RGEditorGUINode::Update(UInt32 frameIndex)
	{

	}

	void RGEditorGUINode::Execute(VulkanCommandBuffer& pBuf)
	{
		mpGUIDriver->OnDraw();

		ImGui::NewFrame();

		bool bShowDemoWindow = true;
		ImGui::ShowDemoWindow(&bShowDemoWindow);

		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
	}

}