#include "Module/renderer_layer.hpp"

// Mythos
#include "Vulkan/mythos_vulkan.hpp"

// temp
#include <Windows.h>

#include "Debug.hpp"
#include "Maths/vectors.hpp"

// --
namespace Mythos
{
	// --

	Mythos::renderer_layer::renderer_layer()
	{
		Debug::log_header("Renderer Layer : Creating the renderer layer");

		vulkan_data_ = Mythos::vulkan::make_unique_vulkan_data(true);

		auto success = false;

		// TODO: get windows handles by event
		auto* hwnd = GetForegroundWindow();
		auto* hmodule = GetModuleHandle(nullptr);

		success = vulkan::create_instance(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_surface(hmodule, hwnd, *vulkan_data_);
		if (!success) return;

		success = vulkan::select_physical_device(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_logical_device(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_swapchain(hwnd, *vulkan_data_);
		if (!success) return;

		success = vulkan::create_image_views(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_render_pass(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_descriptor_set_layout(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_graphics_pipeline(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_command_pool(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_color_resources(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_depth_resources(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_frame_buffers(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_texture_image(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_texture_image_view(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_texture_sampler(*vulkan_data_);
		if (!success) return;

		success = vulkan::load_model(*vulkan_data_); // TODO
		if (!success) return;                        // TODO

		success = vulkan::create_vertex_buffer(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_index_buffer(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_uniform_buffers(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_descriptor_pool(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_descriptor_set(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_command_buffers(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_sync_objects(*vulkan_data_);
		if (!success) return;
	}

	Mythos::renderer_layer::~renderer_layer()
	{
		vulkan::destroy_vulkan_data(*vulkan_data_);
	}

	void Mythos::renderer_layer::update()
	{
	}

	void Mythos::renderer_layer::render()
	{
		// todo : if window can be used then;
		vulkan::draw_frame(GetForegroundWindow(), *vulkan_data_);

		vkDeviceWaitIdle(vulkan_data_->device);
	}
}
