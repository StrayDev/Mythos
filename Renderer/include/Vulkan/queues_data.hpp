#pragma once

// STL
#include <optional>

// Vulkan
#include <vulkan/vulkan_core.h>

namespace Mythos::vulkan
{
	//--

	struct queues_data
	{
		struct graphics
		{
			VkQueue handle = VK_NULL_HANDLE;
			std::optional<uint32_t> family_indices{};
		}
		graphics;

		struct present
		{
			VkQueue handle = VK_NULL_HANDLE;
			std::optional<uint32_t> family_indices{};
		}
		present;

		[[nodiscard]] auto indices_are_valid() const -> bool
		{
			return graphics.family_indices.has_value() && present.family_indices.has_value();
		}
	};
}
