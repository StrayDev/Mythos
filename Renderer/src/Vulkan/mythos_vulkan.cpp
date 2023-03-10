#include "Vulkan/mythos_vulkan.hpp"

// STL
#include <algorithm>
#include <string>
#include <set>

// Mythos
#include "Debug.hpp"
#include "Shader/shader.hpp"
#undef max // need to extract .cpp from debug and set extern in engine dll

// --
namespace Mythos::vulkan
{
	// --

	auto make_unique_vulkan_data(bool set_validation) -> std::unique_ptr<vulkan_data>
	{
		return std::make_unique<vulkan_data>(set_validation);
	}

	auto destroy_vulkan_data(const vulkan_data& vulkan) -> void
	{
		for (const auto framebuffer : vulkan.swapchain.frame_buffers)
		{
			vkDestroyFramebuffer(vulkan.device.logical.handle, framebuffer, nullptr);
		}

		vkDestroyPipeline(vulkan.device.logical.handle, vulkan.graphics_pipeline.handle, nullptr);
		vkDestroyPipelineLayout(vulkan.device.logical.handle, vulkan.graphics_pipeline.layout, nullptr);
		vkDestroyRenderPass(vulkan.device.logical.handle, vulkan.graphics_pipeline.render_pass, nullptr);

		for (const auto& image_view : vulkan.swapchain.image_views)
		{
			vkDestroyImageView(vulkan.device.logical.handle, image_view, nullptr);
		}

		vkDestroySwapchainKHR(vulkan.device.logical.handle, vulkan.swapchain.handle, nullptr);
		vkDestroyDevice(vulkan.device.logical.handle, nullptr);
		vkDestroySurfaceKHR(vulkan.instance.handle, vulkan.surface.handle, nullptr);
		vkDestroyInstance(vulkan.instance.handle, nullptr);

		Debug::log_header("Vulkan objects destroyed");
	}

	static auto get_available_instance_extensions(vulkan_data& vulkan) -> void
	{
		// create extension container
		auto extensions = std::vector<VkExtensionProperties>();
		auto extension_count = uint32_t();

		// get the number of extensions from the instance
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		extensions = std::vector<VkExtensionProperties>(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		// add to the data object
		for (const auto extension : extensions)
		{
			vulkan.instance.available_extensions.push_back(extension);
		}
	}

	static auto log_available_instance_extensions(const vulkan_data& vulkan)
	{
#if _DEBUG
		Debug::log_header("Vulkan available instance extensions : ");
		for (const auto& extension : vulkan.instance.available_extensions)
		{
			Debug::log(" >> " + std::string(extension.extensionName));
		}
#endif
	}

	static auto required_instance_extensions_available(const vulkan_data& vulkan) -> bool
	{
		std::vector<const char*> available_extensions;
		for (const auto [extensionName, specVersion] : vulkan.instance.available_extensions)
		{
			available_extensions.push_back(extensionName);
		}

		for (const auto& required_extension : vulkan.instance.required_extensions)
		{
			if (std::ranges::find(available_extensions, required_extension) == available_extensions.end())
			{
				continue;
			}
			return false;
		}
		return true;
	}

	static auto log_required_instance_extensions(const vulkan_data& vulkan) -> void
	{
#if _DEBUG
		Debug::log_header("Vulkan required instance extensions : ");
		for (const auto extension : vulkan.instance.required_extensions)
		{
			Debug::log(" >> : " + std::string(extension));
		}
#endif
	}

	static auto get_available_instance_layers(vulkan_data& vulkan) -> void
	{
		// get the available layers
		auto layer_count = uint32_t();
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		auto available_layers = std::vector<VkLayerProperties>(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// add them to the vulkan data
		for (const auto& layer : available_layers)
		{
			//std::cout << layer.layerName << '\n';
			if (std::string(layer.layerName).empty()) continue;
			vulkan.instance.available_layers.push_back(layer);
		}

		// setup the validation layers
		auto& create_info = vulkan.instance.create_info;

		if (vulkan.instance.validation_enabled)
		{
			auto& validation_layers = vulkan.instance.validation_layers;
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();

			for (auto layer : validation_layers)
			{
				vulkan.instance.required_layers.push_back(layer);
			}
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}
	}

	static auto log_available_instance_layers(vulkan_data& vulkan) -> void
	{
#if _DEBUG
		Debug::log_header("Vulkan available instance layers : ");
		for (const auto layer : vulkan.instance.available_layers)
		{
			Debug::log(" >> " + std::string(layer.layerName));
		}
#endif
	}

	static auto required_instance_layers_found(const vulkan_data& vulkan) -> bool
	{
		auto available_layers = std::vector<const char*>();
		for (const auto layer : vulkan.instance.available_layers)
		{
			available_layers.push_back(layer.layerName);
		}

		for (const auto& required_layer : vulkan.instance.required_layers)
		{
			if (std::ranges::find(available_layers, required_layer) == available_layers.end())
			{
				continue;
			}
			return false;
		}

		return true;
	}

	static auto log_required_instance_layers(const vulkan_data& vulkan) -> void
	{
#if _DEBUG
		Debug::log_header("Vulkan required instance layers : ");
		for (const auto layer : vulkan.instance.validation_layers)
		{
			Debug::log(" >> : " + std::string(layer));
		}
#endif
	}

	auto create_instance(vulkan_data& vulkan) -> bool
	{
		get_available_instance_extensions(vulkan);
		log_available_instance_extensions(vulkan);

		if (!required_instance_extensions_available(vulkan))
		{
			Debug::error("Vulkan required instance extension missing");
			log_required_instance_extensions(vulkan);
			return false;
		}

		get_available_instance_layers(vulkan);
		log_available_instance_layers(vulkan);

		if (!required_instance_layers_found(vulkan))
		{
			Debug::error("Vulkan required instance layer missing");
			log_required_instance_layers(vulkan);
			return false;
		}

		const auto result = vkCreateInstance(&vulkan.instance.create_info, nullptr, &vulkan.instance.handle);

		if (result != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create instance");
			return false;
		}

		Debug::new_line();
		Debug::log("Vulkan instance created");

		return true;
	}

	auto create_surface(void* hmodule, void* hwnd, vulkan_data& vulkan) -> bool
	{
		auto& create_info = vulkan.surface.create_info;
		create_info.hinstance = static_cast<HMODULE>(hmodule);
		create_info.hwnd = static_cast<HWND>(hwnd);

		const auto result = vkCreateWin32SurfaceKHR(vulkan.instance.handle, &create_info, nullptr,
		                                            &vulkan.surface.handle);

		if (result != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create surface");
			return false;
		}

		Debug::log("Vulkan surface created");
		return true;
	}

	auto get_available_physical_devices(vulkan_data& vulkan) -> bool
	{
		// get the number of physical devices
		auto device_count = uint32_t();
		vkEnumeratePhysicalDevices(vulkan.instance.handle, &device_count, nullptr);

		if (device_count == 0) return false;

		// get list of physical devices
		vulkan.device.physical.available_devices = std::vector<VkPhysicalDevice>(device_count);
		vkEnumeratePhysicalDevices(vulkan.instance.handle, &device_count,
		                           vulkan.device.physical.available_devices.data());
		return true;
	}

	auto device_supports_required_extensions(VkPhysicalDevice physical_device, const vulkan_data& vulkan) -> bool
	{
		auto extension_count = uint32_t();
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(vulkan.device.extensions.begin(), vulkan.device.extensions.end());

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	auto is_physical_device_suitable(VkPhysicalDevice physical_device, vulkan_data& vulkan) -> bool
	{
		const auto valid_queue_indices = vulkan.queues.indices_are_valid();
		const auto valid_extensions = device_supports_required_extensions(physical_device, vulkan);
		const auto valid_present_mode = vulkan.swapchain.is_supported();

		return valid_queue_indices && valid_extensions && valid_present_mode;
	}

	auto set_device_queue_indices(const VkPhysicalDevice& device, vulkan_data& vulkan) -> void
	{
		auto queue_family_count = uint32_t();
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		auto index = 0;

		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				vulkan.queues.graphics.family_indices = index;
			}

			auto presentSupport = static_cast<VkBool32>(false);
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, vulkan.surface.handle, &presentSupport);

			if (presentSupport)
			{
				vulkan.queues.present.family_indices = index;
			}

			index++;
		}
	}

	auto set_device_swapchain_support_data(const VkPhysicalDevice& device, vulkan_data& vulkan) -> void
	{
		// get the support details for the swap chain
		auto& details = vulkan.swapchain.support_details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan.surface.handle, &details.capabilities);

		// get the device format
		auto format_count = uint32_t();
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan.surface.handle, &format_count, nullptr);

		if (format_count != 0)
		{
			details.image_formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan.surface.handle, &format_count,
			                                     details.image_formats.data());
		}

		// get the present mode
		auto present_mode_count = uint32_t();
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan.surface.handle, &present_mode_count, nullptr);

		if (present_mode_count != 0)
		{
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan.surface.handle, &present_mode_count,
			                                          details.present_modes.data());
		}
	}

	auto select_first_suitable_physical_device(vulkan_data& vulkan) -> bool
	{
		for (const auto device : vulkan.device.physical.available_devices)
		{
			set_device_queue_indices(device, vulkan);
			set_device_swapchain_support_data(device, vulkan);

			if (is_physical_device_suitable(device, vulkan))
			{
				vulkan.device.physical.handle = device;
				return true;
			}
		}
		return false;
	}

	auto select_physical_device(vulkan_data& vulkan) -> bool
	{
		if (!get_available_physical_devices(vulkan))
		{
			Debug::error("Vulkan no physical devices available");
			return false;
		}

		if (!select_first_suitable_physical_device(vulkan))
		{
			Debug::error("Vulkan available physical devices are unsuitable");
			return false;
		}

		Debug::log("Vulkan physical device selected");
		return true;
	}

	auto set_device_queue_create_infos(vulkan_data& vulkan) -> void
	{
		auto unique_queue_families = std::set<uint32_t>
		{
			vulkan.queues.graphics.family_indices.value(),
			vulkan.queues.present.family_indices.value(),
		};

		constexpr auto priority = 1.0f;

		// create separate create info for each queue
		for (auto& queue_family : unique_queue_families)
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
			vulkan.device.queue_create_infos.push_back(queue_create_info);
		}
	}

	auto set_logical_device_create_info(vulkan_data& vulkan) -> void
	{
		auto& create_info = vulkan.device.logical.create_info;

		create_info =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(vulkan.device.queue_create_infos.size()),
			.pQueueCreateInfos = vulkan.device.queue_create_infos.data(),
			.enabledLayerCount = 0,
			.enabledExtensionCount = static_cast<uint32_t>(vulkan.device.extensions.size()),
			.ppEnabledExtensionNames = vulkan.device.extensions.data(),
			.pEnabledFeatures = &vulkan.device.features,
		};

		if (vulkan.instance.validation_enabled)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(vulkan.instance.required_layers.size());
			create_info.ppEnabledLayerNames = vulkan.instance.required_layers.data();
		}
	}

	auto create_device_queue_families(vulkan_data& vulkan) -> bool
	{
		vkGetDeviceQueue(vulkan.device.logical.handle, vulkan.queues.graphics.family_indices.value(), 0,
		                 &vulkan.queues.graphics.handle);

		vkGetDeviceQueue(vulkan.device.logical.handle, vulkan.queues.present.family_indices.value(), 0,
		                 &vulkan.queues.present.handle);

		return vulkan.queues.graphics.handle != VK_NULL_HANDLE && vulkan.queues.present.handle != VK_NULL_HANDLE;
	}

	auto create_logical_device(vulkan_data& vulkan) -> bool
	{
		set_device_queue_create_infos(vulkan);

		set_logical_device_create_info(vulkan);

		const auto result = vkCreateDevice(vulkan.device.physical.handle, &vulkan.device.logical.create_info, nullptr,
		                                   &vulkan.device.logical.handle);

		if (result != VK_SUCCESS)
		{
			Debug::error("Vulkan unable to create the logical device");
			return false;
		}

		if (!create_device_queue_families(vulkan))
		{
			Debug::error("Vulkan unable to create the logical device queue families");
			return false;
		}

		Debug::log("Vulkan logical device created");
		return true;
	}

	auto set_swapchain_image_format(vulkan_data& vulkan) -> void
	{
		const auto& available_formats = vulkan.swapchain.support_details.image_formats;

		for (const auto& available_format : available_formats)
		{
			if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				vulkan.swapchain.surface_format = available_format;
			}
		}

		vulkan.swapchain.surface_format = available_formats[0];
	}

	auto set_swapchain_present_mode(vulkan_data& vulkan) -> void
	{
		for (const auto& available_present_mode : vulkan.swapchain.support_details.present_modes)
		{
			if (available_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) continue;
			vulkan.swapchain.present_mode = available_present_mode;
		}
		vulkan.swapchain.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	auto set_swapchain_extents(void* hwnd, vulkan_data& vulkan) -> void
	{
		const auto& capabilities = vulkan.swapchain.support_details.capabilities;

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			vulkan.swapchain.extents = capabilities.currentExtent;
		}
		else
		{
			auto rect = RECT();
			GetClientRect(static_cast<HWND>(hwnd), &rect);

			const int width = rect.right - rect.left;
			const int height = rect.bottom - rect.top;

			auto actual_extent = VkExtent2D
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width,
			                                 capabilities.maxImageExtent.width);
			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height,
			                                  capabilities.maxImageExtent.height);

			vulkan.swapchain.extents = actual_extent;
		}
	}

	auto set_swapchain_create_info(vulkan_data& vulkan) -> void
	{
		const auto support_details = vulkan.swapchain.support_details;
		auto image_count = support_details.capabilities.minImageCount + 1;
		const auto greater_than_max = support_details.capabilities.maxImageCount > 0 && image_count > support_details.
			capabilities.maxImageCount;

		if (greater_than_max)
		{
			image_count = support_details.capabilities.maxImageCount;
		}

		auto& create_info = vulkan.swapchain.create_info;

		create_info = VkSwapchainCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = vulkan.surface.handle,
			.minImageCount = image_count,
			.imageFormat = vulkan.swapchain.surface_format.format,
			.imageColorSpace = vulkan.swapchain.surface_format.colorSpace,
			.imageExtent = vulkan.swapchain.extents,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

			.preTransform = vulkan.swapchain.support_details.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = vulkan.swapchain.present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		const uint32_t queue_family_indices[] =
		{
			vulkan.queues.graphics.family_indices.value(),
			vulkan.queues.present.family_indices.value(),
		};

		if (queue_family_indices[0] != queue_family_indices[1])
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}
	}

	auto create_swapchain(void* hwnd, vulkan_data& vulkan) -> bool
	{
		set_swapchain_image_format(vulkan);

		set_swapchain_present_mode(vulkan);

		set_swapchain_extents(hwnd, vulkan);

		set_swapchain_create_info(vulkan);

		const auto& logical_device = vulkan.device.logical.handle;
		auto& swapchain = vulkan.swapchain;

		const auto result = vkCreateSwapchainKHR(logical_device, &swapchain.create_info, nullptr, &swapchain.handle);
		if (result != VK_SUCCESS)
		{
			Debug::error("failed to create swap chain!");
			return false;
		}
		Debug::log("Vulkan swapchain created");

		auto image_count = swapchain.create_info.minImageCount;
		vkGetSwapchainImagesKHR(logical_device, swapchain.handle, &image_count, nullptr);
		swapchain.images.resize(image_count);
		vkGetSwapchainImagesKHR(logical_device, swapchain.handle, &image_count, swapchain.images.data());

		return true;
	}

	auto create_image_views(vulkan_data& vulkan) -> bool
	{
		auto& swapchain = vulkan.swapchain;
		auto& image_views = vulkan.swapchain.image_views;
		auto& images = vulkan.swapchain.images;

		image_views.resize(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = images[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = swapchain.surface_format.format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			const auto result = vkCreateImageView(vulkan.device.logical.handle, &create_info, nullptr, &image_views[i]);
			if (result != VK_SUCCESS)
			{
				Debug::log("");
				return false;
			}
		}

		Debug::log("Vulkan image views created");
		return true;
	}

	auto create_render_pass(vulkan_data& vulkan) -> bool
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = vulkan.swapchain.surface_format.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// create the render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		const auto success = vkCreateRenderPass(vulkan.device.logical.handle, &renderPassInfo, nullptr,
		                                        &vulkan.graphics_pipeline.render_pass);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create render pass");
			return false;
		}
		Debug::log("Vulkan render pass created");
		return true;
	}

	auto create_shader_module(const std::vector<char>& code, const vulkan_data& vk) -> VkShaderModule
	{
		const auto create_info = VkShaderModuleCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = code.size(),
			.pCode = reinterpret_cast<const uint32_t*>(code.data()),
		};

		VkShaderModule shader_module;
		const auto result = vkCreateShaderModule(vk.device.logical.handle, &create_info, nullptr, &shader_module);
		if (result != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create shader module");
			return VK_NULL_HANDLE;
		}

		return shader_module;
	}

	auto create_graphics_pipeline(vulkan_data& vulkan) -> bool
	{
		Debug::log_header("Creating graphics pipeline :");

		auto vert_shader_code = shader::read_file("../Renderer/shaders/vert.spv");
		auto frag_shader_code = shader::read_file("../Renderer/shaders/frag.spv");

		Debug::new_line();

		if (vert_shader_code.empty() || frag_shader_code.empty())
		{
			Debug::error("Vulkan failed to load graphics pipeline shaders");
			Debug::new_line();
			return false;
		}

		auto vert_shader_module = create_shader_module(vert_shader_code, vulkan);
		auto frag_shader_module = create_shader_module(frag_shader_code, vulkan);

		if (vert_shader_module == VK_NULL_HANDLE || frag_shader_module == VK_NULL_HANDLE)
		{
			Debug::error("Vulkan failed to load graphics pipeline shader modules");
			Debug::new_line();
		}

		// vertex shader 
		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		// fragment shader
		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] =
		{
			vert_shader_stage_info,
			frag_shader_stage_info
		};

		//
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// input assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		// viewport & scissor
		const auto& extents = vulkan.swapchain.extents;
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extents.width);
		viewport.height = static_cast<float>(extents.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extents;

		// dynamic state for things like window resize
		const auto dynamic_states = std::vector<VkDynamicState>
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		// rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		// multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// depth and stencil testing
		// TODO : tutorial comes back to this section

		// colour blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(vulkan.device.logical.handle, &pipelineLayoutInfo, nullptr,
		                           &vulkan.graphics_pipeline.layout) != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create pipeline layout");
			return false;
		}

		// actually create the graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shader_stages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &input_assembly;
		pipelineInfo.pViewportState = &viewport_state;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamic_state;

		pipelineInfo.layout = vulkan.graphics_pipeline.layout;

		pipelineInfo.renderPass = vulkan.graphics_pipeline.render_pass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		const auto success = vkCreateGraphicsPipelines(vulkan.device.logical.handle, VK_NULL_HANDLE, 1, &pipelineInfo,
		                                               nullptr, &vulkan.graphics_pipeline.handle);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create graphics pipeline");
			return false;
		}

		// destroy modules when done : no need to store
		vkDestroyShaderModule(vulkan.device.logical.handle, vert_shader_module, nullptr);
		vkDestroyShaderModule(vulkan.device.logical.handle, frag_shader_module, nullptr);

		Debug::log("Vulkan graphics pipeline created");
		return true;
	}

	auto create_frame_buffers(vulkan_data& vulkan) -> bool
	{
		auto& frame_buffers = vulkan.swapchain.frame_buffers;

		const auto& image_views = vulkan.swapchain.image_views;
		const auto& extent = vulkan.swapchain.extents;

		frame_buffers.resize(image_views.size());

		for (size_t i = 0; i < image_views.size(); i++)
		{
			const VkImageView attachments[] = 
			{
				image_views[i]
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = vulkan.graphics_pipeline.render_pass;
			framebuffer_info.attachmentCount = 1;
			framebuffer_info.pAttachments = attachments;
			framebuffer_info.width = extent.width;
			framebuffer_info.height = extent.height;
			framebuffer_info.layers = 1;

			const auto success =vkCreateFramebuffer(vulkan.device.logical.handle, &framebuffer_info, nullptr, &frame_buffers[i]);
			if (success != VK_SUCCESS)
			{
				Debug::error("Vulkan failed to create a framebuffer");
				return false;
			}
		}
		Debug::log("Vulkan framebuffers created");
		return true;
	}
}
