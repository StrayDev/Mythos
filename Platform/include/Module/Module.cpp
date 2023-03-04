// Headers
#include "Utility/Export.hpp"
#include "Utility/Handles.hpp"

// Include
#include "Module/Module.hpp"
#include "Module/Layer.hpp"

// Utility
#include "Utility/Constants.hpp"
#include "Utility/Macro.hpp"


// Export Module
Export std::unique_ptr<const Mythos::Module> func(MODULE_HANDLE handle)
{
	const auto module = Mythos::Module
	{
		.handle = handle,

		.priority = Mythos::PLATFORM,
		.secondary = Mythos::PLATFORM + 0,

		.name = "Default Windows Platform Module",
		.version = "Version 0.0.0.1",
		.description = "Default platform layer for use with windows OS",

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

	}

	void OnProcessDetach()
	{

	}

#if PLATFORM_WINDOWS

	// entry point
	BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
	{
		switch (reason)
		{

		case DLL_PROCESS_ATTACH:
			OnProcessAttach();
			break;

		case DLL_PROCESS_DETACH:
			OnProcessDetach();
			break;

		default:
			break;

		}

		return TRUE;
	}

#endif

}
