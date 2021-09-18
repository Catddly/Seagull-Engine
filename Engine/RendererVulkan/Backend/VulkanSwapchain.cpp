#include "StdAfx.h"
#include "VulkanSwapchain.h"

#include "System/System.h"
#include "Platform/IOperatingSystem.h"
#include "Platform/Window.h"

#include "VulkanDevice.h"

#ifdef SG_PLATFORM_WINDOWS
#	include <vulkan/vulkan_win32.h>
#endif

namespace SG
{

	VulkanSwapchain::VulkanSwapchain(VkInstance instance)
		:mInstance(instance), mPhysicalDevice(VK_NULL_HANDLE), mLogicalDevice(VK_NULL_HANDLE)
	{
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
	}

	void VulkanSwapchain::BindDevice(VkPhysicalDevice physicalDevice, VkDevice device)
	{
		mPhysicalDevice = physicalDevice;
		mLogicalDevice = device;

		bSwapchainAdequate = true;
	}

	bool VulkanSwapchain::CreateSurface()
	{
#ifdef SG_PLATFORM_WINDOWS
		auto* pOS = SSystem()->GetOS();
		Window* mainWindow = pOS->GetMainWindow();

		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)mainWindow->GetNativeHandle();
		createInfo.hinstance = ::GetModuleHandle(NULL);
		
		if (vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mPresentSurface) != VK_SUCCESS)
			return false;
		return true;
#endif
	}

	void VulkanSwapchain::DestroySurface()
	{
		if (mPresentSurface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(mInstance, mPresentSurface, nullptr);
	}

	bool VulkanSwapchain::CheckSurfacePresentable(VulkanQueue queue)
	{
		// check if the graphic queue can do the presentation job
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, queue.familyIndex, mPresentSurface, &presentSupport);
		if (!presentSupport)
		{
			SG_LOG_ERROR("Current physical device not support surface presentation");
			return false;
		}
		return true;
	}

}