#pragma once

// STL
#include <vector>

// Vulkan
#include <vulkan/vulkan_core.h>

// --
namespace Mythos::vulkan
{
	// --

	struct swapchain_data
	{
		VkSwapchainKHR handle {};
		std::vector<VkFramebuffer> frame_buffers {};
		VkExtent2D extents {};
		VkPresentModeKHR present_mode {};
		VkSurfaceFormatKHR surface_format {};

		std::vector<VkImage> images {};
		std::vector<VkImageView> image_views;

		struct support_data
		{
			VkSurfaceCapabilitiesKHR capabilities {};
			std::vector<VkSurfaceFormatKHR> image_formats {};
			std::vector<VkPresentModeKHR> present_modes {};
		}
		support_details;

		bool is_supported() const
		{
			return !support_details.image_formats.empty() && !support_details.present_modes.empty();
		}

		VkSwapchainCreateInfoKHR create_info {};
	};

}
