#include "StdAfx.h"
#include "RenderContextVk.h"

#include "Common/System/ILog.h"
#include "Common/Platform/Window.h"

#include "RendererVulkan/Queue/QueueVk.h"

#include "Common/Stl/vector.h"
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