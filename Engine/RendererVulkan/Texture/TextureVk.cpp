#include "StdAfx.h"
#include "TextureVk.h"

#include "Common/Render/Renderer.h"
#include "RendererVulkan/Renderer/RendererVk.h"
#include "RendererVulkan/RenderContext/RenderContextVk.h"
#include "RendererVulkan/SwapChain/SwapChainVk.h"

namespace SG
{


	TextureVk::TextureVk(Renderer* pRenderer, const Resolution& res, bool bIsSwapchainImage)
		:mResolution(res), mpRenderer(pRenderer), mbIsSwapchainImage(bIsSwapchainImage)
	{

	}

	TextureVk::~TextureVk()
	{
		if (!mbIsSwapchainImage)
			vkDestroyImage((VkDevice)mpRenderer->GetRenderContext()->GetLogicalDeviceHandle(), mHandle, nullptr);
	}

	Resolution TextureVk::GetResolution() const
	{
		return mResolution;
	}


}
