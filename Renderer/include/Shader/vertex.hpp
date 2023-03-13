#pragma once

// Mythos
#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Maths/vectors.hpp"

namespace Mythos
{
	struct vertex
	{
		float2 pos;
		float3 color;

		static auto get_binding_description() -> VkVertexInputBindingDescription
		{
			auto binding_description = VkVertexInputBindingDescription
			{
				.binding = 0,
				.stride = sizeof(vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			};
			return binding_description;
		};

		static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 2>
		{
			auto attribute_descriptions = std::array<VkVertexInputAttributeDescription, 2>{};

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[0].offset = offsetof(vertex, pos);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(vertex, color);

			return attribute_descriptions;
		}
	};

	const std::vector<vertex> vertices =
	{
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

}
