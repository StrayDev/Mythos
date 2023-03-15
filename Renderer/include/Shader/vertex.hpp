#pragma once

// Mythos
#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>

//#include "Maths/vectors.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace Mythos
{
	struct vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static auto get_binding_description() -> VkVertexInputBindingDescription
		{
			constexpr auto binding_description = VkVertexInputBindingDescription
			{
				.binding = 0,
				.stride = sizeof(vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			};

			return binding_description;
		};

		static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 3>
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(vertex, tex_coord);

			return attributeDescriptions;
		}
	};

	const std::vector<vertex> vertices =
	{
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = 
	{
		0, 1, 2, 2, 3, 0
	};

}
