#include "Module/renderer_layer.hpp"

#include "Debug.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <memory>


Mythos::renderer_layer::renderer_layer()
{
	Debug::log_header("Renderer Layer : Creating the renderer layer");


	// load the Vulkan library and create an instance.
	auto vk_instance = create_vulkan_instance(enable_validation_layers_, validation_layers_);
	vk_instance_ptr_ = std::make_unique<VkInstance>(vk_instance);
	if (*vk_instance_ptr_ == VK_NULL_HANDLE) return;

	// load the vk surface // TODO : remove GetForegroundWindow() and replace with event ?
	auto vk_surface = create_vulkan_surface(GetForegroundWindow(), *vk_instance_ptr_);
	vk_surface_ptr_ = std::make_unique<VkSurfaceKHR>(vk_surface);
	if (*vk_surface_ptr_ == VK_NULL_HANDLE) return;

	// select the physical device
	auto vk_physical_device = select_physical_device(*vk_instance_ptr_);
	vk_physical_device_ptr_ = std::make_unique<VkPhysicalDevice>(vk_physical_device);
	if (*vk_physical_device_ptr_ == VK_NULL_HANDLE) return;

	// create the physical device
	auto vk_logical_device = select_logical_device(*vk_instance_ptr_, *vk_physical_device_ptr_,
	                                               enable_validation_layers_, validation_layers_);
	vk_logical_device_ptr_ = std::make_unique<VkDevice>(vk_logical_device);
	if (*vk_logical_device_ptr_ == VK_NULL_HANDLE) return;

	// Create a swap chain.
	// Create image views for the swap chain images.
	// Create a render pass.
	// Create graphics pipelines.
	// Create a frame buffer for each swap chain image.
	// Record command buffers.
	// Create synchronization objects.
	// Render the frame.

	// Todo : ensure application shuts down correctly when console is closed 
	// Todo : move this to deconstructor
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
	Debug::log("Creating vulkan instance");

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

VkPhysicalDevice Mythos::renderer_layer::select_physical_device(VkInstance vk_instance)
{
	// get the number of physical devices
	auto device_count = uint32_t();
	vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

	if (device_count == 0)
	{
		Debug::error("No valid physical device");
		return VK_NULL_HANDLE; //false
	}

	// get list of physical devices
	auto devices = std::vector<VkPhysicalDevice>(device_count);
	vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());

	// set first suitable device
	for (auto device : devices)
	{
		if (is_device_suitable(device))
		{
			Debug::log("Vulkan physical device selected");
			return device;
		}
	}

	Debug::error("Unable to find suitable physical device");
	return VK_NULL_HANDLE;
}

bool Mythos::renderer_layer::is_device_suitable(VkPhysicalDevice vk_device)
{
	/*auto device_properties = VkPhysicalDeviceProperties();
	auto device_features = VkPhysicalDeviceFeatures();

	vkGetPhysicalDeviceProperties(vk_device, &device_properties);
	vkGetPhysicalDeviceFeatures(vk_device, &device_features);

	// currently just looking for a device that supports geometry shader
	return device_features.geometryShader;*/

	const auto queue = find_queue_families(vk_device);
	return queue.has_value();
}

Mythos::queue_family_indices Mythos::renderer_layer::find_queue_families(VkPhysicalDevice vk_device)
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
			break;
		}

		index++;
	}
	return indices;
}

auto Mythos::renderer_layer::select_logical_device
(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, bool enable_validation_layers, const std::vector<const char*>& validation_layers) -> VkDevice
{
	const auto indices = find_queue_families(vk_physical_device);
	constexpr auto priority = 1.0f;

	// create the create info for the queue
	const auto queue_create_info = VkDeviceQueueCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = indices.graphics_family.value(),
		.queueCount = 1,
		.pQueuePriorities = &priority,
	};

	// set the required device features
	const auto device_features = VkPhysicalDeviceFeatures
	{
	};

	// set the creation options for the logical device
	auto create_info = VkDeviceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_create_info,
		.enabledLayerCount = 0,
		.enabledExtensionCount = 0,
		.pEnabledFeatures = &device_features,
	};

	if (enable_validation_layers)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	// create the logical device
	auto vk_device = static_cast<VkDevice>(VK_NULL_HANDLE);
	const auto result = vkCreateDevice(vk_physical_device, &create_info, nullptr, &vk_device);

	if(result != VK_SUCCESS)
	{
		Debug::error("Unable to create the vk logical device");
		return VK_NULL_HANDLE;
	}

	Debug::log("Vulkan logical device created");
	return vk_device;
}
