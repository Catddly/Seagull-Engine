#include "StdAfx.h"
#include "RenderContextVk.h"

#include "System/ILogger.h"
#include "Platform/Window.h"

#include "RendererVulkan/Queue/QueueVk.h"

#include "Stl/vector.h"
#include <EASTL/set.h>

namespace SG
{

	RenderContextVk::RenderContextVk(Renderer* pRenderer)
		:mpRenderer(pRenderer)
	{
	}

	RenderContextVk::~RenderContextVk()
	{

	}

	SG::Handle RenderContextVk::GetPhysicalDeviceHandle() const
	{
		return mPhysicalDevice;
	}

	SG::Handle RenderContextVk::GetLogicalDeviceHandle() const
	{
		return mLogicalDevice;
	}

	SG::Handle RenderContextVk::GetRenderSurface() const
	{
		return mPresentSurface;
	}

}