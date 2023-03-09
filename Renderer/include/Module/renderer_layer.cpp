#include "Module/renderer_layer.hpp"

// Vulkan
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

// STL
#include <algorithm>
#include <cstdint>
#include <limits> 
#include <memory>
#include <set>

// Mythos
#include "Debug.hpp"
#undef max // bleeding from Windows.h

// --

Mythos::renderer_layer::renderer_layer()
{
	Debug::log_header("Renderer Layer : Creating the renderer layer");

	// // // SETUP // // //
	// TODO : remove GetForegroundWindow() and replace with an event
	const auto window_handle = GetForegroundWindow();

	// load the Vulkan library and create an instance.
	auto vk_instance = create_vulkan_instance(enable_validation_layers_, validation_layers_);
	vk_instance_ptr_ = std::make_unique<VkInstance>(vk_instance);
	if (*vk_instance_ptr_ == VK_NULL_HANDLE) return;

	// load the vk surface 
	auto vk_surface = create_vulkan_surface(window_handle, *vk_instance_ptr_);
	vk_surface_ptr_ = std::make_unique<VkSurfaceKHR>(vk_surface);
	if (*vk_surface_ptr_ == VK_NULL_HANDLE) return;

	// select the physical device
	auto vk_physical_device = select_physical_device(*vk_instance_ptr_, *vk_surface_ptr_, device_extensions_);
	vk_physical_device_ptr_ = std::make_unique<VkPhysicalDevice>(vk_physical_device);
	if (*vk_physical_device_ptr_ == VK_NULL_HANDLE) return;

	// create the physical device
	auto vk_logical_device = create_logical_device(*vk_instance_ptr_, *vk_surface_ptr_, *vk_physical_device_ptr_,
	                                               device_extensions_, enable_validation_layers_, validation_layers_);
	vk_logical_device_ptr_ = std::make_unique<VkDevice>(vk_logical_device);
	if (*vk_logical_device_ptr_ == VK_NULL_HANDLE) return;

	// get the queue for the graphics and present queue handles
	const auto queue_families = find_queue_families(*vk_physical_device_ptr_, *vk_surface_ptr_);

	// get the handle to the graphics queue
	auto vk_graphics_queue = static_cast<VkQueue>(VK_NULL_HANDLE);
	vkGetDeviceQueue(*vk_logical_device_ptr_, queue_families.graphics_family.value(), 0, &vk_graphics_queue);

	vk_graphics_queue_ptr_ = std::make_unique<VkQueue>(vk_graphics_queue);
	if (*vk_graphics_queue_ptr_ == VK_NULL_HANDLE)
	{
		Debug::error("Unable to get handle to the vk graphics queue");
		return;
	}

	// get the handle to the present queue
	auto vk_present_queue = static_cast<VkQueue>(VK_NULL_HANDLE);
	vkGetDeviceQueue(*vk_logical_device_ptr_, queue_families.present_family.value(), 0, &vk_present_queue);

	vk_present_queue_ptr_ = std::make_unique<VkQueue>(vk_present_queue);
	if (*vk_present_queue_ptr_ == VK_NULL_HANDLE)
	{
		Debug::error("Unable to get handle to the vk present queue");
		return;
	}
	Debug::log("Vulkan device queues created");

	// // // PRESENTATION // // //

	// Create a swap chain.
	auto vk_swapchain = create_swap_chain(window_handle, vk_surface, vk_physical_device, vk_logical_device, vk_swapchain_images_, swapchain_image_format_, swapchain_extent_);
	if (vk_swapchain == VK_NULL_HANDLE) return;
	vk_swapchain_ptr_ = std::make_unique<VkSwapchainKHR>(vk_swapchain);

	// Create image views for the swap chain images.
	// Create a render pass.
	// Create graphics pipelines.
	// Create a frame buffer for each swap chain image.
	// Record command buffers.
	// Create synchronization objects.
	// Render the frame.

	// Todo : ensure application shuts down correctly when console is closed 
	// Todo : move this to destructor
	vkDestroySwapchainKHR(*vk_logical_device_ptr_, *vk_swapchain_ptr_, nullptr);
	vkDestroyDevice(*vk_logical_device_ptr_, nullptr);
	vkDestroySurfaceKHR(*vk_instance_ptr_, *vk_surface_ptr_, nullptr);
	vkDestroyInstance(*vk_instance_ptr_, nullptr);
}

Mythos::renderer_layer::~renderer_layer()
{
}

void Mythos::renderer_layer::update()
{
}

void Mythos::renderer_layer::render()
{
}

auto Mythos::renderer_layer::create_vulkan_instance(bool enable_validation,
                                                    const std::vector<const char*>& validation_layers) -> VkInstance
{
	// create the app information
	auto app_info = VkApplicationInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Vulkan Renderer",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	// create extension container
	auto extensions = std::vector<const char*>();
	auto extension_count = uint32_t();

	// get the number of extensions from the instance
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	// get the extension properties
	auto available_extensions = std::vector<VkExtensionProperties>(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

	// add the available extent ions to the list of extensions
	Debug::log_header("Vulkan available extensions; ");
	for (const auto& extension : available_extensions)
	{
		Debug::log(" >> " + std::string(extension.extensionName));
		extensions.push_back(extension.extensionName);
	}

	// set the required extensions
	const auto required_extensions = std::vector<const char*>
	{
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
	};

	// check that the required extensions are present
	Debug::log_header("Required extensions;");
	for (const auto& required_extension : required_extensions)
	{
		if (std::ranges::find(extensions, required_extension) == extensions.end())
		{
			Debug::log(" >> : " + std::string(required_extension));
			continue;
		}
		Debug::error("Required extension not available : " + std::string(required_extension));
		return VK_NULL_HANDLE;
	}

	// create the init data for the instance
	auto create_info = VkInstanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
		.ppEnabledExtensionNames = required_extensions.data()
	};

	// get the count
	auto layer_count = uint32_t();
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	// get the available layers
	auto available_layers = std::vector<VkLayerProperties>(extension_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	// log the layers
	Debug::log_header("Available Layers : ");
	for (const auto& layer : available_layers)
	{
		if (std::string(layer.layerName).empty()) continue;
		Debug::log(" >> " + std::string(layer.layerName));
	}

	// setup the validation layers 
	if (enable_validation)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}
	else
	{
		create_info.enabledLayerCount = 0;
	}

	// create the instance
	auto* instance = static_cast<VkInstance>(VK_NULL_HANDLE);
	const auto result = vkCreateInstance(&create_info, nullptr, &instance);


	if (result != VK_SUCCESS || instance == VK_NULL_HANDLE)
	{
		Debug::error("Failed to create vk_instance");
		return VK_NULL_HANDLE;
	}

	Debug::new_line();
	Debug::log("Vulkan instance created");

	return instance;
}

VkSurfaceKHR Mythos::renderer_layer::create_vulkan_surface(HWND window_handle, VkInstance vk_instance)
{
	auto create_info = VkWin32SurfaceCreateInfoKHR
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = GetModuleHandle(nullptr),
		.hwnd = window_handle,
	};

	auto* surface = static_cast<VkSurfaceKHR>(VK_NULL_HANDLE);
	const auto result = vkCreateWin32SurfaceKHR(vk_instance, &create_info, nullptr, &surface);

	if (result != VK_SUCCESS)
	{
		Debug::error("Failed to create vk_surface");
		return VK_NULL_HANDLE;
	}

	Debug::log("Vulkan surface created");

	return surface;
}

VkPhysicalDevice Mythos::renderer_layer::select_physical_device(VkInstance vk_instance, VkSurfaceKHR vk_surface,
                                                                std::vector<const char*> device_extensions)
{
	// get the number of physical devices
	auto device_count = uint32_t();
	vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

	if (device_count == 0)
	{
		Debug::error("No valid physical device");
		return VK_NULL_HANDLE;
	}

	// get list of physical devices
	auto devices = std::vector<VkPhysicalDevice>(device_count);
	vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());

	// set first suitable device
	for (auto device : devices)
	{
		if (is_device_suitable(device, vk_surface, device_extensions))
		{
			Debug::log("Vulkan physical device selected");
			return device;
		}
	}

	Debug::error("Unable to find suitable physical device");
	return VK_NULL_HANDLE;
}

auto Mythos::renderer_layer::create_logical_device(VkInstance vk_instance, VkSurfaceKHR vk_surface,
                                                   VkPhysicalDevice vk_physical_device,
                                                   std::vector<const char*> device_extensions, bool enable_validation,
                                                   const std::vector<const char*>& validation_layers) -> VkDevice
{
	// get the info indices for the queue families
	const auto indices = find_queue_families(vk_physical_device, vk_surface);
	auto unique_queue_families = std::set<uint32_t>
	{
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	constexpr auto priority = 1.0f;

	auto queue_create_infos = std::vector<VkDeviceQueueCreateInfo>();

	// create separate create info for each queue
	for (auto queue_family : unique_queue_families)
	{
		// create the create info for the queue
		auto queue_create_info = VkDeviceQueueCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queue_family,
			.queueCount = 1,
			.pQueuePriorities = &priority,
		};

		// add to the list of create infos
		queue_create_infos.push_back(queue_create_info);
	}

	// set the required device features
	const auto device_features = VkPhysicalDeviceFeatures
	{
	};

	// set the creation options for the logical device
	auto create_info = VkDeviceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
		.pQueueCreateInfos = queue_create_infos.data(),
		.enabledLayerCount = 0,
		.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
		.ppEnabledExtensionNames = device_extensions.data(),
		.pEnabledFeatures = &device_features,
	};

	if (enable_validation)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	// create the logical device
	auto vk_device = static_cast<VkDevice>(VK_NULL_HANDLE);
	const auto result = vkCreateDevice(vk_physical_device, &create_info, nullptr, &vk_device);

	if (result != VK_SUCCESS)
	{
		Debug::error("Unable to create the vk logical device");
		return VK_NULL_HANDLE;
	}

	Debug::log("Vulkan logical device created");
	return vk_device;
}

auto Mythos::renderer_layer::is_device_suitable(VkPhysicalDevice vk_device, VkSurfaceKHR vk_surface,
                                                std::vector<const char*> device_extensions) -> bool
{
	const auto queue_families_complete = find_queue_families(vk_device, vk_surface).is_complete();
	const auto device_extensions_supported = check_device_extension_support(vk_device, device_extensions);

	bool swap_chain_adequate = false;
	if (device_extensions_supported)
	{
		const auto swap_chain_support = query_swap_chain_support(vk_surface, vk_device);
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}

	return queue_families_complete && device_extensions_supported && swap_chain_adequate;
}

auto Mythos::renderer_layer::check_device_extension_support(VkPhysicalDevice vk_device,
                                                            std::vector<const char*> device_extensions) -> bool
{
	auto extension_count = uint32_t();
	vkEnumerateDeviceExtensionProperties(vk_device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(vk_device, nullptr, &extension_count, available_extensions.data());

	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for (const auto& extension : available_extensions)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

Mythos::queue_family_indices Mythos::renderer_layer::find_queue_families(
	VkPhysicalDevice vk_device, VkSurfaceKHR vk_surface)
{
	auto queue_family_count = uint32_t();
	vkGetPhysicalDeviceQueueFamilyProperties(vk_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vk_device, &queue_family_count, queue_families.data());

	auto indices = queue_family_indices();

	auto index = 0;
	for (const auto& queue_family : queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_family = index;
		}

		auto presentSupport = static_cast<VkBool32>(false);
		vkGetPhysicalDeviceSurfaceSupportKHR(vk_device, index, vk_surface, &presentSupport);

		if (presentSupport)
		{
			indices.present_family = index;
		}

		index++;
	}

	return indices;
}

auto Mythos::renderer_layer::create_swap_chain(HWND window_handle, VkSurfaceKHR vk_surface,
	VkPhysicalDevice vk_physical_device, VkDevice vk_logical_device, std::vector<VkImage>& vk_swapchain_images,
	VkFormat& vk_format, VkExtent2D& vk_extent_2D) -> VkSwapchainKHR
{
	// get the format presentation mode and window extents
	const auto swap_chain_support = query_swap_chain_support(vk_surface, vk_physical_device);
	const auto surface_format = choose_swap_surface_format(swap_chain_support.formats);
	const auto present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
	const auto extent = choose_swap_extent(window_handle, swap_chain_support.capabilities);

	// set image count to min +1 or max
	auto image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) 
	{
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	// create info for the swap chain
	auto create_info = VkSwapchainCreateInfoKHR
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vk_surface,
		.minImageCount = image_count,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

		.preTransform = swap_chain_support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	const auto indices = find_queue_families(vk_physical_device, vk_surface);
	const uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

	if (indices.graphics_family != indices.present_family) 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // Optional
		create_info.pQueueFamilyIndices = nullptr; // Optional
	}

	// create the swap chain
	auto swap_chain = static_cast<VkSwapchainKHR>(VK_NULL_HANDLE);
	const auto result = vkCreateSwapchainKHR(vk_logical_device, &create_info, nullptr, &swap_chain);
	if (result != VK_SUCCESS) 
	{
		Debug::error("failed to create swap chain!");
		return VK_NULL_HANDLE;
	}
	Debug::log("Vulkan swapchain created");

	// create swap chain images
	vkGetSwapchainImagesKHR(vk_logical_device, swap_chain, &image_count, nullptr);
	vk_swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(vk_logical_device, swap_chain, &image_count, vk_swapchain_images.data());

	vk_format = surface_format.format;
	vk_extent_2D = extent;
	
	return swap_chain;
}

auto Mythos::renderer_layer::query_swap_chain_support(VkSurfaceKHR vk_surface,
                                                      VkPhysicalDevice vk_physical_device) -> swap_chain_support_details
{
	// get the support details for the swap chain
	auto details = swap_chain_support_details();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &details.capabilities);

	// get the device format
	auto format_count = uint32_t();
	vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, nullptr);

	if (format_count != 0)
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, details.formats.data());
	}

	// get the present mode
	auto present_mode_count = uint32_t();
	vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, nullptr);

	if (present_mode_count != 0)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count,
		                                          details.present_modes.data());
	}

	return details;
}

auto Mythos::renderer_layer::choose_swap_surface_format(
	const std::vector<VkSurfaceFormatKHR>& available_formats) -> VkSurfaceFormatKHR
{
	for (const auto& available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace ==
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available_format;
		}
	}
	return available_formats[0];
}

auto Mythos::renderer_layer::choose_swap_present_mode(
	const std::vector<VkPresentModeKHR>& available_present_modes) -> VkPresentModeKHR
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) continue;
		return available_present_mode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

auto Mythos::renderer_layer::choose_swap_extent(HWND window_handle, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		auto rect = RECT();
		GetClientRect(window_handle, &rect);

		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;

		auto actual_extent = VkExtent2D
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}
