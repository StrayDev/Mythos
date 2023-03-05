// Headers
#include "Utility/Export.hpp"
#include "Utility/Handles.hpp"

// Include
#include "Module/Module.hpp"
#include "Module/ILayer.hpp"

// Utility
#include <iostream>

#include "Layer.hpp"
#include "Utility/Constants.hpp"
#include "Utility/Macro.hpp"


// Export Module
MODULE_API std::unique_ptr<const Mythos::Module> func(MODULE_HANDLE handle)
{
	const auto module = Mythos::Module
	{
		.handle = handle,

		.priority = Mythos::EVENT,
		.secondary = Mythos::EVENT + 0,

		.name = "Default Event Module",
		.version = "Version 0.0.0.1",
		.description = "Default event layer",

		.dll_path = FILE_PATH,
		.dll_name = FILE_NAME,
		.dll_date = FILE_DATE,
		.dll_time = FILE_TIME,

		.MakeUniqueLayer = []() -> std::unique_ptr<Mythos::ILayer>
		{
			return std::make_unique<Mythos::Layer>();
		},

	};

	return std::make_unique<const Mythos::Module>(module);
}

// --
namespace Mythos
{

	void OnProcessAttach()
	{
		std::cout << "Event Module Attached";
	}

	void OnProcessDetach()
	{
		std::cout << "Event Module Detached";

	}

}
