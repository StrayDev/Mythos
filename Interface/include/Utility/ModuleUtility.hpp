#pragma once


// STL
#include <memory>
#include <string>

// Constants
#include "Constants.hpp"

// Class
#include "Module/Module.hpp"

// Utility
#include "FileUtility.hpp"
#include "InterfaceUtility.hpp"
#include "StringUtility.hpp"


// --
namespace Mythos::Utility
{

    void* LoadModuleHandle(const std::string& file_name)
    {
	    const auto handle = LoadLibrary(StringToWString(file_name).c_str());

        if (handle == nullptr)
        {
            std::cout << "Failed to load module : " << file_name << '\n';
            return nullptr;
        }

        return handle;
    }

	std::unique_ptr<Module> LoadModule(const std::string& file_name, void* handle)
    {
        // First, define a function pointer type for MakeUnique
        using func = std::unique_ptr<Module>(*)(void*);

        // Get the function pointer using GetProcAddress()
        const auto proc = GetProcAddress(static_cast<HMODULE>(handle), "func");

        // Check we have the correct proc
        if(proc == nullptr)
        {
            std::cout << "Module pointer is null : " << __LINE__ << ' ' << __FILE__ << '\n';
            return nullptr;
        }

        // cast it to a usable type
        auto invoke = reinterpret_cast<func>(proc);

        // create the module object
        auto module = invoke(handle);

        // check the object is valid
        if(module == nullptr)
        {
            std::cout << "Failed to create Module Object : " << __LINE__ << ' ' << __FILE__ << '\n';
            return nullptr;
        }

        return std::move(module);
    }

	void UnloadModule(const std::unique_ptr<Module>& module)
	{
		auto result = FreeLibrary(module->handle);

    	if(result = false)
        {
            std::cout << "Failed to unload module : " << __LINE__ << ' ' << __FILE__ << '\n';
        }

	}

	bool TryLoadModules(std::vector<std::unique_ptr<Module>>& list)
    {
        list = std::vector<std::unique_ptr<Module>>();

        // get the list of .dll file names
        const auto directory = GetExePath();
        const auto files = GetFilesByExtention(directory, ".dll");

        for (const auto& file_name : files)
        {
            // create the module
            auto* handle = LoadModuleHandle(file_name);
            auto  module = LoadModule(file_name, handle);

            // skip if null
            if (module == nullptr) continue;

            // add to the stack
            list.push_back(std::move(module));
        }

        // early exit if no modules
        if (list.empty())
        {
            std::cout << "Failed to load any modules : " << __LINE__ << ' ' << __FILE__ << '\n';
            return false;
        }

        // sort modules by priority order
        std::ranges::sort(list, PrioritySort);

        // check that stack contains at least 1 platform module
        if (list.front()->priority != PLATFORM)
        {
            std::cout << "Unable to load Platform Module" << '\n' << __LINE__ << " : " << __FILE__ << '\n';
            return false;
        }

        return true;
    }

	bool TryCreateLayers(std::vector<std::unique_ptr<Module>>& modules, std::vector<std::unique_ptr<ILayer>>& layers)
    {
        // create layer stack
        layers = std::vector<std::unique_ptr<ILayer>>();

        // create the layers in priority order
        for (const auto& module : modules)
        {
            auto layer = module->MakeUniqueLayer();

            if (layer == nullptr)
            {
                // if module == PLATFORM - assert - module not able to create layer
                std::cout << "Module failed to Generate Layer" << '\n' << __LINE__ << " : " << __FILE__ << '\n';
                continue;
            }

            layers.push_back(std::move(layer));
        }

        return !layers.empty();
    }



}


