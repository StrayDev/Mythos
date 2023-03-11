#pragma once

#ifndef _WINDEF_

typedef void* HANDLE;

struct HWND__;
typedef HWND__* HWND;

struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

struct HMONITOR__;
typedef HMONITOR__* HMONITOR;


typedef const wchar_t* LPCWSTR;
typedef unsigned long  DWORD;

struct _SECURITY_ATTRIBUTES;
typedef _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;

#endif

// Vulkan
#include <Vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>

// STL
#include <functional>
#include <optional>

// -- 
namespace Mythos::vulkan
{
	// --

	struct vulkan_data
	{
		explicit vulkan_data(bool enable_validation = true) : validation_enabled(enable_validation) {}
		~vulkan_data() = default;

		// validation
		const bool validation_enabled;

		const std::vector<const char*> validation_layers
		{
			"VK_LAYER_KHRONOS_validation",
		};

		// application
		VkApplicationInfo application_info
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Vulkan Renderer",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0
		};

		std::vector<const char*> required_layers{};
		std::vector<VkLayerProperties> available_layers{};

		std::vector<VkExtensionProperties> extensions{};
		std::vector<VkExtensionProperties> available_extensions{};

		const std::vector<const char*> required_extensions
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
		};

		// instance data
		VkInstance instance = VK_NULL_HANDLE;

		VkInstanceCreateInfo instance_create_info
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &application_info,
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data()
		};

		// surface data
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkWin32SurfaceCreateInfoKHR surface_create_info
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		};

		// device data
		VkDevice device = VK_NULL_HANDLE;
		VkDeviceCreateInfo device_create_info {};
		std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos{};

		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures physical_device_features{};
		std::vector<VkPhysicalDevice> available_devices {};

		const std::vector<const char*> device_extensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// queues
		VkQueue graphics_queue = VK_NULL_HANDLE;
		std::optional<uint32_t> graphics_queue_family_indices{};

		VkQueue present_queue = VK_NULL_HANDLE;
		std::optional<uint32_t> present_queue_family_indices{};

		// swapchain
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkSwapchainCreateInfoKHR swapchain_create_info{};

		VkExtent2D swapchain_extents{};
		VkPresentModeKHR swapchain_present_mode{};
		VkSurfaceFormatKHR swapchain_surface_format{};

		std::vector<VkImage> images {};
		std::vector<VkImageView> image_views {};
		std::vector<VkFramebuffer> frame_buffers {};

		struct swapchain_support_details
		{
			VkSurfaceCapabilitiesKHR capabilities{};
			std::vector<VkSurfaceFormatKHR> image_formats{};
			std::vector<VkPresentModeKHR> present_modes{};
		}
		swapchain_support_details;

		//graphics pipeline
		VkPipeline graphics_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout{};
		VkRenderPass render_pass{};

		// command
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;

		// sync objects
		VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
		VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
		VkFence in_flight_fence = VK_NULL_HANDLE;

	};

}
