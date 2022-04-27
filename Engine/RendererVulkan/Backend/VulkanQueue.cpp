#include "StdAfx.h"
#include "VulkanQueue.h"

#include "volk.h"

namespace SG
{

	void VulkanQueue::WaitIdle() const
	{
		vkQueueWaitIdle(handle);
	}

}