#pragma once


// STL
#include <memory>
#include <string>

// Constants
#include "Constants.hpp"

// Class
#include "Module/Module.hpp"

// Mythos
#include "Debug.hpp"
#include "FileUtility.hpp"
#include "InterfaceUtility.hpp"
#include "StringUtility.hpp"


// --
namespace Mythos::Utility
{

    MODULE_HANDLE LoadModuleLibrary(const std::string& file_name)
    {
	    const auto handle = LoadLibrary(StringToWString(file_name).c_str());

        if (handle == nullptr)
        {
            Debug::error("Failed to load module : " + file_name);
            return nullptr;
        }

        Debug::log("Library Successfully Loaded : " + file_name);
        return handle;
    }

	std::unique_ptr<Module> LoadModule(MODULE_HANDLE handle)
    {
        // First, define a function pointer type for MakeUnique
        using func = std::unique_ptr<Module>(*)(void*);

        // Get the function pointer using GetProcAddress()
        const auto proc = GetProcAddress(handle, "func");

        // Check we have the correct proc
        if(proc == nullptr)
        {
            Debug::warn("Module pointer is null");
            return nullptr;
        }

        // cast it to a usable type
        auto invoke = reinterpret_cast<func>(proc);

        // create the module object
        auto module = invoke(handle);

        // check the object is valid
        if(module == nullptr)
        {
            Debug::warn("Failed to create module object");
            return nullptr;
        }

        return std::move(module);
    }

	void UnloadModule(const std::unique_ptr<Module>& module)
	{
		auto result = FreeLibrary(module->handle);

    	if(result = false)
        {
            Debug::error("Failed to unload module");
        }

	}

	bool TryLoadModules(std::vector<std::unique_ptr<Module>>& list)
    {
        // init list
        list = std::vector<std::unique_ptr<Module>>();

        // get the list of .dll file names
        const auto directory = GetExePath();
        const auto files = GetFilesByExtention(directory, ".dll");

        for (const auto& file_name : files)
        {
            // create the module
            auto* handle = LoadModuleLibrary(file_name);
            auto  module = LoadModule(handle);

            // skip if null
            if (module == nullptr) continue;

            // add to the stack
            list.push_back(std::move(module));
        }

        // early exit if no modules
        if (list.empty())
        {
            Debug::error("Failed to load any modules : ");
            return false;
        }

        // sort modules by priority order
        std::ranges::sort(list, PrioritySort);

        // check that stack contains at least 1 platform module
        if (list.front()->priority != EVENT)
        {
            Debug::log("Unable to load platform module");
            return false;
        }

        return true;
    }

	bool TryCreateLayers(std::vector<std::unique_ptr<Module>>& modules, std::vector<std::unique_ptr<layer>>& layers)
    {
        // create layer stack
        layers = std::vector<std::unique_ptr<layer>>();

        // create the layers in priority order
        for (const auto& module : modules)
        {
            auto layer = module->MakeUniqueLayer();

            if (layer == nullptr)
            {
                // if module == PLATFORM - assert - module not able to create layer
                Debug::log("Failed to create layer from module");
                continue;
            }

            layers.push_back(std::move(layer));
        }

        return !layers.empty();
    }



}


