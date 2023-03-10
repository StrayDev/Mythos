#pragma once

// STL
#include <vector>

// Vulkan
#include <vulkan/vulkan_core.h>

namespace Mythos::vulkan
{
	//--

	struct graphics_pipeline_data
	{
		VkPipeline handle;
		VkPipelineLayout layout {};
		VkRenderPass render_pass {};


	};

}