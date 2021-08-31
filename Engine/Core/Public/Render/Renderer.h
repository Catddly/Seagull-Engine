#pragma once

#include "Defs/Defs.h"
#include "Core/Config.h"
#include "Base/BasicTypes.h"
#include "System/IModule.h"

namespace SG
{

#define SG_SWAPCHAIN_IMAGE_COUNT 2 // temporary, should be moved to renderer

	struct Queue;
	struct RenderContext;
	struct SwapChain;
	interface Renderer : public IModule
	{
		virtual ~Renderer() = default;

		//SG_CORE_API virtual Handle GetRendererInstance() const = 0;
		SG_CORE_API virtual RenderContext* GetRenderContext() const = 0;
		// TODO: Graphic queue can be multiple.
		SG_CORE_API virtual Queue* GetGraphicQueue() const = 0;
		SG_CORE_API virtual Queue* GetPresentQueue() const = 0;

		SG_CORE_API virtual SwapChain* GetSwapChain() const = 0;
	};

}