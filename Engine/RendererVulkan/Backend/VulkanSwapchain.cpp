#include "StdAfx.h"
#include "VulkanSwapchain.h"

#include "System/System.h"
#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"

#include "VulkanDevice.h"

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <vulkan/vulkan_win32.h>
#endif

namespace SG
{

	VulkanSwapchain::VulkanSwapchain(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
		:mInstance(instance), mPhysicalDevice(physicalDevice), mLogicalDevice(device)
	{
		bSwapchainAdequate = true;
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		if (mPresentSurface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(mInstance, mPresentSurface, nullptr);
	}

	bool VulkanSwapchain::CreateSurface(VulkanQueue graphicQueue)
	{
		if (!bSwapchainAdequate)
			return false;

#ifdef SG_PLATFORM_WINDOWS
		auto* pOS = SSystem()->GetOS();
		Window* mainWindow = pOS->GetMainWindow();

		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);

		if (!vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mPresentSurface) != VK_SUCCESS)
		{
			// check if the graphic queue can do the presentation job
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, graphicQueue.familyIndex, mPresentSurface, &presentSupport);
			if (!presentSupport)
			{
				SG_LOG_ERROR("Current physical device not support surface presentation");
				return false;
			}
			return true;
		}
		else
		{
			return false;
		}
#endif
	}

}