#pragma once

// Vulkan
#include <Vulkan/vulkan_core.h>

// --
namespace Mythos::vulkan
{
	// --

	struct instance_data
	{
		VkInstance handle = VK_NULL_HANDLE;

		VkApplicationInfo app_info
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Vulkan Renderer",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0
		};

		std::vector<VkExtensionProperties> extensions {};
		std::vector<VkExtensionProperties> available_extensions {};

		const std::vector<const char*> required_extensions
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
		};
		
		VkInstanceCreateInfo create_info
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &app_info,
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data()
		};

		const bool validation_enabled;

		const std::vector<const char*> validation_layers
		{
			"VK_LAYER_KHRONOS_validation",
		};

		std::vector<const char*> required_layers{};
		std::vector<VkLayerProperties> available_layers{};

	};
}
