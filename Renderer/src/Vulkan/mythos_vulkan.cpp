#include "Vulkan/mythos_vulkan.hpp"

// STL
#include <algorithm>
#include <chrono>
#include <string>
#include <set>

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Mythos
#include "Debug.hpp"
#include "Shader/shader.hpp"
#include "Shader/uniform_buffer_object.hpp"
#include "Shader/vertex.hpp"
#undef max // need to extract .cpp from debug and set extern in engine dll

// --
namespace Mythos::vulkan
{
	// --

	auto make_unique_vulkan_data(bool set_validation) -> std::unique_ptr<vulkan_data>
	{
		return std::make_unique<vulkan_data>(set_validation);
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
			vulkan.available_extensions.push_back(extension);
		}
	}

	static auto log_available_instance_extensions(const vulkan_data& vulkan)
	{
#if _DEBUG
		Debug::log_header("Vulkan available instance extensions : ");
		for (const auto& extension : vulkan.available_extensions)
		{
			Debug::log(" >> " + std::string(extension.extensionName));
		}
#endif
	}

	static auto required_instance_extensions_available(const vulkan_data& vulkan) -> bool
	{
		std::vector<const char*> available_extensions;
		for (const auto [extensionName, specVersion] : vulkan.available_extensions)
		{
			available_extensions.push_back(extensionName);
		}

		for (const auto& required_extension : vulkan.required_extensions)
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
		for (const auto extension : vulkan.required_extensions)
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
			vulkan.available_layers.push_back(layer);
		}

		// setup the validation layers
		auto& create_info = vulkan.instance_create_info;

		if (vulkan.validation_enabled)
		{
			auto& validation_layers = vulkan.validation_layers;
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();

			for (auto layer : validation_layers)
			{
				vulkan.required_layers.push_back(layer);
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
		for (const auto layer : vulkan.available_layers)
		{
			Debug::log(" >> " + std::string(layer.layerName));
		}
#endif
	}

	static auto required_instance_layers_found(const vulkan_data& vulkan) -> bool
	{
		auto available_layers = std::vector<const char*>();
		for (const auto layer : vulkan.available_layers)
		{
			available_layers.push_back(layer.layerName);
		}

		for (const auto& required_layer : vulkan.required_layers)
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
		for (const auto layer : vulkan.validation_layers)
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

		const auto result = vkCreateInstance(&vulkan.instance_create_info, nullptr, &vulkan.instance);

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
		auto& create_info = vulkan.surface_create_info;
		create_info.hinstance = static_cast<HMODULE>(hmodule);
		create_info.hwnd = static_cast<HWND>(hwnd);

		const auto result = vkCreateWin32SurfaceKHR(vulkan.instance, &create_info, nullptr,
		                                            &vulkan.surface);

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
		vkEnumeratePhysicalDevices(vulkan.instance, &device_count, nullptr);

		if (device_count == 0) return false;

		// get list of physical devices
		vulkan.available_devices = std::vector<VkPhysicalDevice>(device_count);
		vkEnumeratePhysicalDevices(vulkan.instance, &device_count,
		                           vulkan.available_devices.data());
		return true;
	}

	auto device_supports_required_extensions(VkPhysicalDevice physical_device, const vulkan_data& vulkan) -> bool
	{
		auto extension_count = uint32_t();
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(vulkan.device_extensions.begin(), vulkan.device_extensions.end());

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	auto swapchain_is_supported(const vulkan_data& vulkan) -> bool
	{
		return !vulkan.swapchain_support_details.image_formats.empty() && !vulkan.swapchain_support_details.
			present_modes.empty();
	}


	auto queue_indices_are_valid(const vulkan_data& vulkan) -> bool
	{
		return vulkan.graphics_queue_family_indices.has_value() && vulkan.present_queue_family_indices.has_value();
	}

	auto is_physical_device_suitable(VkPhysicalDevice physical_device, const vulkan_data& vulkan) -> bool
	{
		const auto valid_queue_indices = queue_indices_are_valid(vulkan);
		const auto valid_extensions = device_supports_required_extensions(physical_device, vulkan);
		const auto valid_present_mode = swapchain_is_supported(vulkan);

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
				vulkan.graphics_queue_family_indices = index;
			}

			auto presentSupport = static_cast<VkBool32>(false);
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, vulkan.surface, &presentSupport);

			if (presentSupport)
			{
				vulkan.present_queue_family_indices = index;
			}

			index++;
		}
	}

	auto set_device_swapchain_support_data(const VkPhysicalDevice& device, vulkan_data& vulkan) -> void
	{
		// get the support details for the swap chain
		auto& details = vulkan.swapchain_support_details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan.surface, &details.capabilities);

		// get the device format
		auto format_count = uint32_t();
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan.surface, &format_count, nullptr);

		if (format_count != 0)
		{
			details.image_formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan.surface, &format_count,
			                                     details.image_formats.data());
		}

		// get the present mode
		auto present_mode_count = uint32_t();
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan.surface, &present_mode_count, nullptr);

		if (present_mode_count != 0)
		{
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan.surface, &present_mode_count,
			                                          details.present_modes.data());
		}
	}

	auto select_first_suitable_physical_device(vulkan_data& vulkan) -> bool
	{
		for (const auto device : vulkan.available_devices)
		{
			set_device_queue_indices(device, vulkan);
			set_device_swapchain_support_data(device, vulkan);

			if (is_physical_device_suitable(device, vulkan))
			{
				vulkan.physical_device = device;
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
			vulkan.graphics_queue_family_indices.value(),
			vulkan.present_queue_family_indices.value(),
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
			vulkan.device_queue_create_infos.push_back(queue_create_info);
		}
	}

	auto set_logical_device_create_info(vulkan_data& vulkan) -> void
	{
		auto& create_info = vulkan.device_create_info;

		create_info =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(vulkan.device_queue_create_infos.size()),
			.pQueueCreateInfos = vulkan.device_queue_create_infos.data(),
			.enabledLayerCount = 0,
			.enabledExtensionCount = static_cast<uint32_t>(vulkan.device_extensions.size()),
			.ppEnabledExtensionNames = vulkan.device_extensions.data(),
			.pEnabledFeatures = &vulkan.physical_device_features,
		};

		if (vulkan.validation_enabled)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(vulkan.required_layers.size());
			create_info.ppEnabledLayerNames = vulkan.required_layers.data();
		}
	}

	auto create_device_queue_families(vulkan_data& vulkan) -> bool
	{
		vkGetDeviceQueue(vulkan.device, vulkan.graphics_queue_family_indices.value(), 0, &vulkan.graphics_queue);

		vkGetDeviceQueue(vulkan.device, vulkan.present_queue_family_indices.value(), 0, &vulkan.present_queue);

		return vulkan.graphics_queue != VK_NULL_HANDLE && vulkan.present_queue != VK_NULL_HANDLE;
	}

	auto create_logical_device(vulkan_data& vulkan) -> bool
	{
		set_device_queue_create_infos(vulkan);

		set_logical_device_create_info(vulkan);

		const auto result = vkCreateDevice(vulkan.physical_device, &vulkan.device_create_info, nullptr,
		                                   &vulkan.device);

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
		const auto& available_formats = vulkan.swapchain_support_details.image_formats;

		for (const auto& available_format : available_formats)
		{
			if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				vulkan.swapchain_surface_format = available_format;
			}
		}

		vulkan.swapchain_surface_format = available_formats[0];
	}

	auto set_swapchain_present_mode(vulkan_data& vulkan) -> void
	{
		for (const auto& available_present_mode : vulkan.swapchain_support_details.present_modes)
		{
			if (available_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) continue;
			vulkan.swapchain_present_mode = available_present_mode;
		}
		vulkan.swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	auto set_swapchain_extents(void* hwnd, vulkan_data& vulkan) -> void
	{
		const auto& capabilities = vulkan.swapchain_support_details.capabilities;

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			vulkan.swapchain_extents = capabilities.currentExtent;
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

			vulkan.swapchain_extents = actual_extent;
		}
	}

	auto set_swapchain_create_info(vulkan_data& vulkan) -> void
	{
		const auto support_details = vulkan.swapchain_support_details;
		auto image_count = support_details.capabilities.minImageCount + 1;
		const auto greater_than_max = support_details.capabilities.maxImageCount > 0 && image_count > support_details.
			capabilities.maxImageCount;

		if (greater_than_max)
		{
			image_count = support_details.capabilities.maxImageCount;
		}

		auto& create_info = vulkan.swapchain_create_info;

		create_info = VkSwapchainCreateInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = vulkan.surface,
			.minImageCount = image_count,
			.imageFormat = vulkan.swapchain_surface_format.format,
			.imageColorSpace = vulkan.swapchain_surface_format.colorSpace,
			.imageExtent = vulkan.swapchain_extents,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

			.preTransform = vulkan.swapchain_support_details.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = vulkan.swapchain_present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		const uint32_t queue_family_indices[] =
		{
			vulkan.graphics_queue_family_indices.value(),
			vulkan.present_queue_family_indices.value(),
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

		const auto result = vkCreateSwapchainKHR(vulkan.device, &vulkan.swapchain_create_info, nullptr,
		                                         &vulkan.swapchain);
		if (result != VK_SUCCESS)
		{
			Debug::error("failed to create swap chain!");
			return false;
		}
		Debug::log("Vulkan swapchain created");

		auto image_count = vulkan.swapchain_create_info.minImageCount;
		vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &image_count, nullptr);
		vulkan.images.resize(image_count);
		vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &image_count, vulkan.images.data());

		return true;
	}

	auto create_image_views(vulkan_data& vulkan) -> bool
	{
		vulkan.image_views.resize(vulkan.images.size());

		auto component_mapping = VkComponentMapping
		{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		};

		auto image_subresource_range = VkImageSubresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		for (size_t i = 0; i < vulkan.images.size(); i++)
		{
			auto create_info = VkImageViewCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = vulkan.images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = vulkan.swapchain_surface_format.format,
				.components = component_mapping,
				.subresourceRange = image_subresource_range,
			};

			const auto result = vkCreateImageView(vulkan.device, &create_info, nullptr, &vulkan.image_views[i]);
			if (result != VK_SUCCESS)
			{
				Debug::log("Vulkan failed to create image view");
				return false;
			}
		}

		Debug::log("Vulkan image views created");
		return true;
	}

	auto create_render_pass(vulkan_data& vulkan) -> bool
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = vulkan.swapchain_surface_format.format;
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

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// create the render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		const auto success = vkCreateRenderPass(vulkan.device, &renderPassInfo, nullptr, &vulkan.render_pass);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create render pass");
			return false;
		}
		Debug::log("Vulkan render pass created");
		return true;
	}

	auto create_descriptor_set_layout(vulkan_data& vulkan) -> bool
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(vulkan.device, &layoutInfo, nullptr, &vulkan.descriptor_set_layout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}



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
		const auto result = vkCreateShaderModule(vk.device, &create_info, nullptr, &shader_module);
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
		auto binding_description = vertex::get_binding_description();
		auto attribute_descriptions = vertex::get_attribute_descriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_create_info.vertexBindingDescriptionCount = 1;
		vertex_input_create_info.pVertexBindingDescriptions = &binding_description;
		vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

		// input assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		// viewport & scissor
		const auto& extents = vulkan.swapchain_extents;
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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &vulkan.descriptor_set_layout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		auto success = vkCreatePipelineLayout(vulkan.device, &pipelineLayoutInfo, nullptr, &vulkan.pipeline_layout);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create pipeline layout");
			return false;
		}

		// actually create the graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shader_stages;

		pipelineInfo.pVertexInputState = &vertex_input_create_info;
		pipelineInfo.pInputAssemblyState = &input_assembly;
		pipelineInfo.pViewportState = &viewport_state;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamic_state;

		pipelineInfo.layout = vulkan.pipeline_layout;

		pipelineInfo.renderPass = vulkan.render_pass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		success = vkCreateGraphicsPipelines(vulkan.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
		                                    &vulkan.graphics_pipeline);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create graphics pipeline");
			return false;
		}

		// destroy modules when done : no need to store
		vkDestroyShaderModule(vulkan.device, vert_shader_module, nullptr);
		vkDestroyShaderModule(vulkan.device, frag_shader_module, nullptr);

		Debug::log("Vulkan graphics pipeline created");
		return true;
	}

	auto create_frame_buffers(vulkan_data& vulkan) -> bool
	{
		auto& frame_buffers = vulkan.frame_buffers;

		const auto& image_views = vulkan.image_views;
		const auto& extent = vulkan.swapchain_extents;

		frame_buffers.resize(image_views.size());

		for (size_t i = 0; i < image_views.size(); i++)
		{
			const VkImageView attachments[] =
			{
				image_views[i]
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = vulkan.render_pass;
			framebuffer_info.attachmentCount = 1;
			framebuffer_info.pAttachments = attachments;
			framebuffer_info.width = extent.width;
			framebuffer_info.height = extent.height;
			framebuffer_info.layers = 1;

			const auto success = vkCreateFramebuffer(vulkan.device, &framebuffer_info, nullptr, &frame_buffers[i]);
			if (success != VK_SUCCESS)
			{
				Debug::error("Vulkan failed to create a framebuffer");
				return false;
			}
		}
		Debug::log("Vulkan framebuffers created");
		return true;
	}

	auto create_command_pool(vulkan_data& vulkan) -> bool
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = vulkan.graphics_queue_family_indices.value();

		const auto success = vkCreateCommandPool(vulkan.device, &pool_info, nullptr,
		                                         &vulkan.command_pool);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to create command pool!");
			return false;
		}

		Debug::log("Vulkan created command pool");
		return true;
	}

	auto create_command_buffers(vulkan_data& vulkan) -> bool
	{
		vulkan.command_buffers.resize(vulkan.MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vulkan.command_pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(vulkan.command_buffers.size());

		const auto success = vkAllocateCommandBuffers(vulkan.device, &allocInfo, vulkan.command_buffers.data());
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to allocate command buffers");
		}

		Debug::log("Vulkan allocated command buffers");
		return true;
	}

	auto create_sync_objects(vulkan_data& vulkan) -> bool
	{
		vulkan.image_available_semaphores.resize(vulkan.MAX_FRAMES_IN_FLIGHT);
		vulkan.render_finished_semaphores.resize(vulkan.MAX_FRAMES_IN_FLIGHT);
		vulkan.in_flight_fences.resize(vulkan.MAX_FRAMES_IN_FLIGHT);

		auto result = false;
		auto result_2 = false;

		for (auto i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++)
		{
			result = vkCreateSemaphore(vulkan.device, &vulkan.semaphore_create_info, nullptr,
			                           &vulkan.image_available_semaphores[i]);
			result_2 = vkCreateSemaphore(vulkan.device, &vulkan.semaphore_create_info, nullptr,
			                             &vulkan.render_finished_semaphores[i]);
			if (result != VK_SUCCESS || result_2 != VK_SUCCESS)
			{
				Debug::error("Vulkan failed to create the semaphores");
				return false;
			}
		}

		for (auto i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++)
		{
			result = vkCreateFence(vulkan.device, &vulkan.fence_create_info, nullptr, &vulkan.in_flight_fences[i]);
			if (result != VK_SUCCESS)
			{
				Debug::error("Vulkan failed to create the fences");
				return false;
			}
		}

		Debug::log("Vulkan created the synchronization objects");
		return true;
	}

	auto record_command_buffer(vulkan_data& vulkan, uint32_t image_index) -> void
	{
		const auto& command_buffer = vulkan.command_buffers[vulkan.current_frame];
		const auto& render_pass = vulkan.render_pass;
		const auto& frame_buffers = vulkan.frame_buffers;
		const auto& swapchain_extent = vulkan.swapchain_extents;


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		auto success = vkBeginCommandBuffer(command_buffer, &beginInfo);
		if (success != VK_SUCCESS)
		{
			Debug::error("Vulkan failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;
		renderPassInfo.framebuffer = frame_buffers[image_index];

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapchain_extent;

		VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clear_color;

		// begin render pass
		vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// bind the graphics pipeline
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.graphics_pipeline);

		// these get set here because we have set them to be dynamic
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain_extent.width);
		viewport.height = static_cast<float>(swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		auto& buffer = vulkan.vertex_buffer;

		VkBuffer vertexBuffers[] = {vulkan.vertex_buffer};
		VkDeviceSize offsets[] = {0};

		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(command_buffer, vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.pipeline_layout, 0, 1, &vulkan.descriptor_sets[vulkan.current_frame], 0, nullptr);

		// draw command
		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		// end the render pass
		vkCmdEndRenderPass(command_buffer);

		success = vkEndCommandBuffer(command_buffer);
		if (success != VK_SUCCESS)
		{
			Debug::error("failed to record command buffer!");
		}
	}

	auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, const vulkan_data& vulkan) -> uint32_t
	{
		auto mem_properties = VkPhysicalDeviceMemoryProperties();
		vkGetPhysicalDeviceMemoryProperties(vulkan.physical_device, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	auto create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                   VkDeviceMemory& buffer_memory, vulkan_data& vulkan) -> bool
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(vulkan.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vulkan.device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties, vulkan);

		if (vkAllocateMemory(vulkan.device, &allocInfo, nullptr, &buffer_memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
			return false;
		}

		vkBindBufferMemory(vulkan.device, buffer, buffer_memory, 0);
		return true;
	}

	void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, vulkan_data& vulkan)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vulkan.command_pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(vulkan.device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(vulkan.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(vulkan.graphics_queue);

		vkFreeCommandBuffers(vulkan.device, vulkan.command_pool, 1, &commandBuffer);
	}

	auto create_vertex_buffer(vulkan_data& vulkan) -> bool
	{
		const auto buffer_size = VkDeviceSize{sizeof(vertices[0]) * vertices.size()};

		constexpr auto staging_usage = VkBufferUsageFlags{VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		constexpr auto staging_properties = VkMemoryPropertyFlags{
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		auto success = create_buffer(buffer_size, staging_usage, staging_properties, staging_buffer,
		                             staging_buffer_memory, vulkan);

		if (!success) return false;

		void* data;
		vkMapMemory(vulkan.device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(vulkan.device, staging_buffer_memory);

		constexpr auto vertex_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		constexpr auto vertex_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		success = create_buffer(buffer_size, vertex_usage, vertex_properties, vulkan.vertex_buffer,
		                        vulkan.vertex_buffer_memory, vulkan);

		if (!success) return false;

		copy_buffer(staging_buffer, vulkan.vertex_buffer, buffer_size, vulkan);

		vkDestroyBuffer(vulkan.device, staging_buffer, nullptr);
		vkFreeMemory(vulkan.device, staging_buffer_memory, nullptr);

		return true;
	}

	auto create_index_buffer(vulkan_data& vulkan) -> bool
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		              stagingBufferMemory, vulkan);

		void* data;
		vkMapMemory(vulkan.device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(vulkan.device, stagingBufferMemory);

		create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan.index_buffer, vulkan.index_buffer_memory, vulkan);

		copy_buffer(stagingBuffer, vulkan.index_buffer, bufferSize, vulkan);

		vkDestroyBuffer(vulkan.device, stagingBuffer, nullptr);
		vkFreeMemory(vulkan.device, stagingBufferMemory, nullptr);

		return true;
	}

	auto create_uniform_buffers(vulkan_data& vulkan) -> bool
	{
		VkDeviceSize bufferSize = sizeof(uniform_buffer_object);

		vulkan.uniform_buffers.resize(vulkan.MAX_FRAMES_IN_FLIGHT);
		vulkan.uniform_buffers_memory.resize(vulkan.MAX_FRAMES_IN_FLIGHT);
		vulkan.uniform_buffers_mapped.resize(vulkan.MAX_FRAMES_IN_FLIGHT);

		constexpr auto usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		constexpr auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		auto success = false;
		for (size_t i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++) 
		{
			success = create_buffer(bufferSize, usage, properties, vulkan.uniform_buffers[i], vulkan.uniform_buffers_memory[i], vulkan);
			if (!success) return false;
			vkMapMemory(vulkan.device, vulkan.uniform_buffers_memory[i], 0, bufferSize, 0, &vulkan.uniform_buffers_mapped[i]);
		}

		return true;
	}

	auto create_descriptor_pool(vulkan_data& vulkan) -> bool
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(vulkan.MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(vulkan.MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(vulkan.device, &poolInfo, nullptr, &vulkan.descriptor_pool) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor pool!");
			return false;
		}

		return true;
	}

	auto create_descriptor_set(vulkan_data& vulkan) -> bool
	{
		std::vector<VkDescriptorSetLayout> layouts(vulkan.MAX_FRAMES_IN_FLIGHT, vulkan.descriptor_set_layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = vulkan.descriptor_pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan.MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		vulkan.descriptor_sets.resize(vulkan.MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(vulkan.device, &allocInfo, vulkan.descriptor_sets.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
			return false;
		}

		for (size_t i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++) 
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = vulkan.uniform_buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(uniform_buffer_object);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = vulkan.descriptor_sets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(vulkan.device, 1, &descriptorWrite, 0, nullptr);
		}

		return true;
	}

	void update_uniform_buffer(vulkan_data& vulkan)
	{
		static auto start_time = std::chrono::high_resolution_clock::now();

		const auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

		uniform_buffer_object ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj  = glm::perspective(glm::radians(45.0f), vulkan.swapchain_extents.width / static_cast<float>(vulkan.swapchain_extents.height), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		memcpy(vulkan.uniform_buffers_mapped[vulkan.current_frame], &ubo, sizeof(ubo));

	}

	auto draw_frame(void* hwnd, vulkan_data& vulkan) -> void
	{
		const auto i = vulkan.current_frame;
		auto image_index = uint32_t();

		vkWaitForFences(vulkan.device, 1, &vulkan.in_flight_fences[i], VK_TRUE, UINT64_MAX);
		auto result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX,
		                                    vulkan.image_available_semaphores[i], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain(hwnd, vulkan);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		vkResetFences(vulkan.device, 1, &vulkan.in_flight_fences[i]);

		vkResetCommandBuffer(vulkan.command_buffers[i], 0);
		record_command_buffer(vulkan, image_index);

		update_uniform_buffer(vulkan);

		const VkSemaphore wait_semaphores[] = {vulkan.image_available_semaphores[i]};
		const VkSemaphore signal_semaphores[] = {vulkan.render_finished_semaphores[i]};

		constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		auto submit_info = VkSubmitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &vulkan.command_buffers[i],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signal_semaphores,
		};

		result = vkQueueSubmit(vulkan.graphics_queue, 1, &submit_info, vulkan.in_flight_fences[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan failed to submit draw command buffer");
		}

		VkSwapchainKHR swapChains[] = {vulkan.swapchain};

		const auto present_info = VkPresentInfoKHR
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signal_semaphores,
			.swapchainCount = 1,
			.pSwapchains = swapChains,
			.pImageIndices = &image_index,
			.pResults = nullptr,
		};

		result = vkQueuePresentKHR(vulkan.present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkan.frame_buffer_resized)
		{
			vulkan.frame_buffer_resized = false;
			recreate_swapchain(hwnd, vulkan);
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		vulkan.current_frame = (i + 1) % vulkan.MAX_FRAMES_IN_FLIGHT;
	}

	auto clean_up_swapchain(vulkan_data& vulkan) -> void
	{
		for (size_t i = 0; i < vulkan.frame_buffers.size(); i++)
		{
			vkDestroyFramebuffer(vulkan.device, vulkan.frame_buffers[i], nullptr);
		}

		for (size_t i = 0; i < vulkan.image_views.size(); i++)
		{
			vkDestroyImageView(vulkan.device, vulkan.image_views[i], nullptr);
		}

		vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, nullptr);
	}

	auto recreate_swapchain(void* hwnd, vulkan_data& vulkan) -> void
	{
		vkDeviceWaitIdle(vulkan.device);

		clean_up_swapchain(vulkan);

		create_swapchain(hwnd, vulkan);
		create_image_views(vulkan);
		create_frame_buffers(vulkan);
	}

	auto destroy_vulkan_data(vulkan_data& vulkan) -> void
	{
		clean_up_swapchain(vulkan);

		for (size_t i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroyBuffer(vulkan.device, vulkan.uniform_buffers[i], nullptr);
			vkFreeMemory(vulkan.device, vulkan.uniform_buffers_memory[i], nullptr);
		}

		vkDestroyDescriptorPool(vulkan.device, vulkan.descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(vulkan.device, vulkan.descriptor_set_layout, nullptr);

		vkDestroyBuffer(vulkan.device, vulkan.index_buffer, nullptr);
		vkFreeMemory(vulkan.device, vulkan.index_buffer_memory, nullptr);

		vkDestroyBuffer(vulkan.device, vulkan.vertex_buffer, nullptr);
		vkFreeMemory(vulkan.device, vulkan.vertex_buffer_memory, nullptr);

		vkDestroyPipeline(vulkan.device, vulkan.graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(vulkan.device, vulkan.pipeline_layout, nullptr);

		vkDestroyRenderPass(vulkan.device, vulkan.render_pass, nullptr);

		for (auto i = 0; i < vulkan.MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(vulkan.device, vulkan.image_available_semaphores[i], nullptr);
			vkDestroySemaphore(vulkan.device, vulkan.render_finished_semaphores[i], nullptr);
			vkDestroyFence(vulkan.device, vulkan.in_flight_fences[i], nullptr);
		}

		vkDestroyCommandPool(vulkan.device, vulkan.command_pool, nullptr);

		vkDestroyDevice(vulkan.device, nullptr);

		if (vulkan.validation_enabled)
		{
			//DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, nullptr);
		vkDestroyInstance(vulkan.instance, nullptr);

		Debug::log_header("Vulkan objects destroyed");
	}
}
