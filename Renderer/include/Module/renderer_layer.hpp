#pragma once

// STL
#include <memory>
#include <vector>

// Vulkan
#include <vulkan/vulkan_core.h>

// Mythos
#include <optional>

#include "Module/layer.hpp"
#include "Utility/Handles.hpp"

// --
namespace Mythos
{
	//--

	struct queue_family_indices
	{
		std::optional<uint32_t> graphics_family;
		bool has_value() const { return graphics_family.has_value(); }
	};

	// --
	class renderer_layer : public layer
	{
	public:
		renderer_layer();
		~renderer_layer() override;

		void update() override;
		void render() override;

		static auto create_vulkan_instance(bool enable_validation,
		                                   const std::vector<const char*>& validation_layers) -> VkInstance;

		static auto create_vulkan_surface(HWND window_handle, VkInstance vk_instance) -> VkSurfaceKHR;
		static auto select_physical_device(VkInstance vk_instance) -> VkPhysicalDevice;

		static auto select_logical_device(VkInstance vk_instance, VkPhysicalDevice vk_physical_device,
		                                  bool enable_validation,
		                                  const std::vector<const char*>& validation_layers) -> VkDevice;

		static auto is_device_suitable(VkPhysicalDevice vk_device) -> bool;
		static auto find_queue_families(VkPhysicalDevice vk_device) -> queue_family_indices;

	private:
		bool enable_validation_layers_ = true;

		std::vector<const char*> validation_layers_
		{
			"VK_LAYER_KHRONOS_validation",
		};

		std::unique_ptr<VkInstance> vk_instance_ptr_;
		std::unique_ptr<VkSurfaceKHR> vk_surface_ptr_;
		std::unique_ptr<VkPhysicalDevice> vk_physical_device_ptr_;
		std::unique_ptr<VkDevice> vk_logical_device_ptr_;
	};
}
