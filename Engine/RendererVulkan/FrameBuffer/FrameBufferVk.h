#pragma once

#include "Common/Render/FrameBuffer.h"

#include "Common/Stl/vector.h"

#include <vulkan/vulkan_core.h>

namespace SG
{

	struct Renderer;
	struct Texture;
	class FrameBufferVk : public FrameBuffer
	{
	public:
		FrameBufferVk(Renderer* pRenderer);
		~FrameBufferVk();
	private:
		Renderer* mpRenderer = nullptr;
		vector<VkFramebuffer> mBuffers;
		vector<VkImageView>   mSwapChainImagaViews;
	};

}