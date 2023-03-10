#include "Module/renderer_layer.hpp"

// Mythos
#include "Vulkan/mythos_vulkan.hpp"

// temp
#include <Windows.h>

#include "Debug.hpp"

// --
namespace Mythos
{
	// --

	Mythos::renderer_layer::renderer_layer()
	{
		Debug::log_header("Renderer Layer : Creating the renderer layer");

		vulkan_data_ = Mythos::vulkan::make_unique_vulkan_data(true);

		/*vulkan_data_->set_destructor_callback( [&]()
		{
			// TODO: uncomment this line once application shutdown is properly managed
			// passing in an empty lambda will disable the warning msg 
			// vulkan::destroy_vulkan_data(*vulkan_data_);
		});*/

		auto success = false;

		success = vulkan::create_instance(*vulkan_data_);
		if (!success) return;

		// TODO: get windows handles by event
		auto* hwnd = GetForegroundWindow();
		auto* hmodule = GetModuleHandle(nullptr);

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

		success = vulkan::create_graphics_pipeline(*vulkan_data_);
		if (!success) return;

		success = vulkan::create_frame_buffers(*vulkan_data_);
		if (!success) return;

		vulkan::destroy_vulkan_data(*vulkan_data_);

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

}

