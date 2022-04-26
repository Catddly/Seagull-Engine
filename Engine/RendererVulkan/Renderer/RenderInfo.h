#pragma once

#include "Base/BasicTypes.h"

#include "RendererVulkan/Backend/VulkanCommand.h"

namespace SG
{

	struct DrawInfo
	{
		VulkanCommandBuffer* pCmd;
		UInt32               frameIndex;
	};

}