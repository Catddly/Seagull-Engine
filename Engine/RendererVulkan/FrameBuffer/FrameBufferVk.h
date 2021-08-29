#pragma once

#include "Render/FrameBuffer.h"

#include "Stl/vector.h"

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