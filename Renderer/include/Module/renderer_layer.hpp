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
		std::optional<uint32_t> present_family;

		auto is_complete() const -> bool
		{
			return graphics_family.has_value() && present_family.has_value();
		};
	};

	struct swap_chain_support_details
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	// --

	class renderer_layer : public layer
	{
	public:
		renderer_layer();
		~renderer_layer() override;

		void update() override;
		void render() override;

		static auto create_vulkan_instance(bool enable_validation, const std::vector<const char*>& validation_layers) -> VkInstance;

		static auto create_vulkan_surface(HWND window_handle, VkInstance vk_instance) -> VkSurfaceKHR;

		static auto select_physical_device(VkInstance vk_instance, VkSurfaceKHR vk_surface, std::vector<const char*> device_extensions) -> VkPhysicalDevice;

		static auto create_logical_device(VkInstance vk_instance, VkSurfaceKHR vk_surface, VkPhysicalDevice vk_physical_device,
		                                  std::vector<const char*> device_extensions, bool enable_validation,
		                                  const std::vector<const char*>& validation_layers) -> VkDevice;

		static auto is_device_suitable(VkPhysicalDevice vk_device, VkSurfaceKHR vk_surface, std::vector<const char*> device_extensions) -> bool;

		static auto check_device_extension_support(VkPhysicalDevice vk_device, std::vector<const char*> device_extensions) -> bool;

		static auto find_queue_families(VkPhysicalDevice vk_device, VkSurfaceKHR vk_surface) -> queue_family_indices;

		static auto create_swap_chain(HWND window_handle, VkSurfaceKHR vk_surface, VkPhysicalDevice vk_physical_device, VkDevice vk_logical_device, std::vector<VkImage>& vk_swapchain_images, VkFormat& vk_format, VkExtent2D& vk_extent_2D) -> VkSwapchainKHR;

		static auto query_swap_chain_support(VkSurfaceKHR vk_surface, VkPhysicalDevice device) -> swap_chain_support_details;

		static auto choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) -> VkSurfaceFormatKHR;

		static auto choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) -> VkPresentModeKHR;

		static auto choose_swap_extent(HWND window_handle, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D;


	private:
		const bool enable_validation_layers_ = true;

		const std::vector<const char*> validation_layers_
		{
			"VK_LAYER_KHRONOS_validation",
		};

		// set the device extensions to be used
		const std::vector<const char*> device_extensions_ =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::unique_ptr<VkInstance> vk_instance_ptr_;
		std::unique_ptr<VkSurfaceKHR> vk_surface_ptr_;

		std::unique_ptr<VkPhysicalDevice> vk_physical_device_ptr_;
		std::unique_ptr<VkDevice> vk_logical_device_ptr_;

		std::unique_ptr<VkQueue> vk_graphics_queue_ptr_;
		std::unique_ptr<VkQueue> vk_present_queue_ptr_;

		std::unique_ptr<VkSwapchainKHR> vk_swapchain_ptr_;
		std::vector<VkImage> vk_swapchain_images_;
		VkFormat swapchain_image_format_;
		VkExtent2D swapchain_extent_;
	};

}
