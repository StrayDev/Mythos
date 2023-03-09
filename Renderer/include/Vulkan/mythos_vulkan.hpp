#pragma once

// STL
#include <memory>

// Mythos
#include "vulkan_data.hpp"

// -
namespace Mythos::vulkan
{
	// --

	auto make_unique_vulkan_data(bool set_validation = false) -> std::unique_ptr<vulkan_data>;

	auto destroy_vulkan_data(vulkan_data& vulkan) -> void;

	auto create_instance(vulkan_data& vulkan) -> bool;

	auto create_surface(void* hmodule, void* hwnd, vulkan_data& vulkan) -> bool;

	auto select_physical_device(vulkan_data& vulkan) -> bool;

	auto create_logical_device(vulkan_data& vulkan) -> bool;

	auto create_swapchain(void* hwnd, vulkan_data& vulkan) -> bool;

}