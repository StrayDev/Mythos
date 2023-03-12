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
		vulkan_data(bool enable_validation = true);

		~vulkan_data() = default;

		// validation
		const bool validation_enabled;

		// constants
		const int MAX_FRAMES_IN_FLIGHT = 2;

		// application
		VkApplicationInfo application_info = {};

		// handles
		VkDevice device = VK_NULL_HANDLE;
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

		VkQueue present_queue = VK_NULL_HANDLE;
		VkQueue graphics_queue = VK_NULL_HANDLE;

		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkPipeline graphics_pipeline = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> command_buffers = {};

		std::vector<VkFence> in_flight_fences = {};
		std::vector<VkSemaphore> image_available_semaphores = {};
		std::vector<VkSemaphore> render_finished_semaphores = {};

		// create info
		VkDeviceCreateInfo device_create_info = {};
		VkInstanceCreateInfo instance_create_info = {};
		VkSwapchainCreateInfoKHR swapchain_create_info{};
		VkWin32SurfaceCreateInfoKHR surface_create_info = {};

		VkFenceCreateInfo fence_create_info = {};
		VkSemaphoreCreateInfo semaphore_create_info = {};


		std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos = {};

		// layers
		std::vector<const char*> validation_layers = {};
		std::vector<const char*> required_layers = {};
		std::vector<VkLayerProperties> available_layers = {};

		// extensions
		std::vector<const char*> required_extensions = {};
		std::vector<VkExtensionProperties> extensions = {};
		std::vector<VkExtensionProperties> available_extensions = {};

		// device
		std::vector<const char*> device_extensions = {};
		std::vector<VkPhysicalDevice> available_devices = {};
		VkPhysicalDeviceFeatures physical_device_features = {};

		// queues family indices
		std::optional<uint32_t> graphics_queue_family_indices{};
		std::optional<uint32_t> present_queue_family_indices{};

		// swapchain
		VkExtent2D swapchain_extents{};
		VkPresentModeKHR swapchain_present_mode{};
		VkSurfaceFormatKHR swapchain_surface_format{};

		struct swapchain_support_details
		{
			VkSurfaceCapabilitiesKHR capabilities{};
			std::vector<VkPresentModeKHR> present_modes{};
			std::vector<VkSurfaceFormatKHR> image_formats{};
		}
		swapchain_support_details;

		// graphics pipeline
		int current_frame = 0;
		bool frame_buffer_resized = false;

		std::vector<VkImage> images{};
		std::vector<VkImageView> image_views{};
		std::vector<VkFramebuffer> frame_buffers{};

		//graphics pipeline
		VkPipelineLayout pipeline_layout{};
		VkRenderPass render_pass{};

	};

	inline vulkan_data::vulkan_data(bool enable_validation): validation_enabled(enable_validation)
	{
		application_info = VkApplicationInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Vulkan Renderer",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0
		};

		validation_layers = std::vector<const char*>
		{
			"VK_LAYER_KHRONOS_validation",
		};

		required_extensions = std::vector<const char*>
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
		};

		instance_create_info = VkInstanceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &application_info,
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data()
		};

		surface_create_info = VkWin32SurfaceCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		};

		device_extensions = std::vector<const char*>
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		semaphore_create_info = VkSemaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		fence_create_info = VkFenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
	}
}
