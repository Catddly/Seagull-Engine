#pragma once

#include "Common/Config.h"

namespace SG
{

	typedef void* Handle;

	struct Queue;
	struct RenderContext;
	struct Renderer
	{
		virtual ~Renderer() = default;

		SG_COMMON_API virtual bool OnInit() = 0;
		SG_COMMON_API virtual void OnShutdown() = 0;

		//SG_COMMON_API virtual Handle GetRendererInstance() const = 0;
		SG_COMMON_API virtual RenderContext* GetRenderContext() const = 0;
		// TODO: Graphic queue can be multiple.
		SG_COMMON_API virtual Queue* GetGraphicQueue() const = 0;
		SG_COMMON_API virtual Queue* GetPresentQueue() const = 0;

	};

}