#pragma once

// STL
#include <memory>

// Mythos
#include "vulkan_data.hpp"

// -
namespace Mythos::vulkan
{
	// TODO : does not yet support window resize and minimization
	// TODO : need to implement event system and window events
	// --
	
	auto make_unique_vulkan_data(bool set_validation = false) -> std::unique_ptr<vulkan_data>;

	auto create_instance(vulkan_data& vulkan) -> bool;

	auto create_surface(void* hmodule, void* hwnd, vulkan_data& vulkan) -> bool;

	auto select_physical_device(vulkan_data& vulkan) -> bool;

	auto create_logical_device(vulkan_data& vulkan) -> bool;

	auto create_swapchain(void* hwnd, vulkan_data& vulkan) -> bool;

	auto create_image_views(vulkan_data& vulkan) -> bool;

	auto create_render_pass(vulkan_data& vulkan) -> bool;

	auto create_graphics_pipeline(vulkan_data& vulkan) -> bool;

	auto create_frame_buffers(vulkan_data& vulkan) -> bool;

	auto create_command_pool(vulkan_data& vulkan) -> bool;

	auto create_command_buffers(vulkan_data& vulkan) -> bool;

	auto create_sync_objects(vulkan_data& vulkan) -> bool;

	auto draw_frame(void* hwnd, vulkan_data& vulkan) -> void;

	auto recreate_swapchain(void* hwnd, vulkan_data& vulkan) -> void;

	auto destroy_vulkan_data(vulkan_data& vulkan) -> void;

}