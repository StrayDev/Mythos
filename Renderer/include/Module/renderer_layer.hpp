#pragma once

// STL
#include <memory>

// Mythos
#include "Module/layer.hpp"
#include "Vulkan/vulkan_data.hpp"

// --
namespace Mythos
{
	//--

	class renderer_layer : public layer
	{
	public:
		renderer_layer();
		~renderer_layer() override;

		void update() override;
		void render() override;

	private:
		std::unique_ptr<vulkan::vulkan_data> vulkan_data_ {};

	};

}
