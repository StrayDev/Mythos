#include "Vulkan/vulkan_data.hpp"

// Mythos
#include "Debug.hpp"

// --
namespace Mythos::vulkan
{
	// --

	vulkan_data::vulkan_data(bool validation_enabled) : instance{ .validation_enabled = validation_enabled, }
	{
		Debug::log("Vulkan data container created");
	}

	vulkan_data::~vulkan_data()
	{
		if (!destructor_callback_)
		{
			Debug::warn("Vulkan handle objects are not currently being destroyed");
			return;
		}
		destructor_callback_();
		Debug::log("Vulkan handle objects destroyed");
	}

	void vulkan_data::set_destructor_callback(const std::function<void()>& callback)
	{
		destructor_callback_ = callback;
	}
}
