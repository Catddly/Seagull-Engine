#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"
#include "Common/System/IModule.h"

namespace SG
{

#define SG_SWAPCHAIN_IMAGE_COUNT 2 // temporary, should be moved to renderer

	struct Queue;
	struct RenderContext;
	struct SwapChain;
	struct Renderer : public IModule
	{
		virtual ~Renderer() = default;

		//SG_COMMON_API virtual Handle GetRendererInstance() const = 0;
		SG_COMMON_API virtual RenderContext* GetRenderContext() const = 0;
		// TODO: Graphic queue can be multiple.
		SG_COMMON_API virtual Queue* GetGraphicQueue() const = 0;
		SG_COMMON_API virtual Queue* GetPresentQueue() const = 0;

		SG_COMMON_API virtual SwapChain* GetSwapChain() const = 0;
	};

}