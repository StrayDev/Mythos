#pragma once

// STL
#include <vector>

// Vulkan
#include <vulkan/vulkan_core.h>

namespace Mythos::vulkan
{
	//--

	struct device_data
	{
		const std::vector<const char*> extensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkPhysicalDeviceFeatures features {};

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos {};

		struct physical
		{
			VkPhysicalDevice handle = VK_NULL_HANDLE;
			std::vector<VkPhysicalDevice> available_devices {};
		}
		physical;

		struct logical
		{
			VkDevice handle = VK_NULL_HANDLE;
			VkDeviceCreateInfo create_info {};
		}
		logical;

	};

}