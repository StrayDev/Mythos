#pragma once

// STL
#include <functional>

// Mythos
#include "device_data.hpp"
#include "graphics_pipeline.hpp"
#include "instance_data.hpp"
#include "queues_data.hpp"
#include "swapchain_data.hpp"
#include "surface_data.hpp"

// -- 
namespace Mythos::vulkan
{
	// --

	struct vulkan_data
	{
		explicit vulkan_data(bool enable_validation = true);

		~vulkan_data();

		void set_destructor_callback(const std::function<void()>& callback);

		instance_data instance {};
		surface_data surface {};
		device_data device {};
		queues_data queues{};
		swapchain_data swapchain {};
		graphics_pipeline_data graphics_pipeline{};

	private:
		std::function<void()> destructor_callback_ = nullptr;

	};

}
