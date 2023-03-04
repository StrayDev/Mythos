#pragma once

// Microsoft
#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#include <Windows.h>
#endif

// STL
#include <array>
#include <filesystem>
#include <iostream>
#include <string>

// --
namespace Mythos::Utility
{

    std::string GetExePath()
    {
        // create the buffer
        constexpr auto max = 260;
        auto buffer = std::array<wchar_t, max>();

        // get the exe file path
        GetModuleFileName(nullptr, buffer.data(), max);

        // convert to a file path object 
        const auto path = std::filesystem::path(buffer.data());

        // retrieve the string 
        return path.parent_path().string();
    }


    std::vector<std::string> GetFilesByExtention(const std::string& directory, const std::string& extension)
    {
        // create container
        auto dll_names = std::vector<std::string>();

        // find DLLs in folder
        for (auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_directory()) continue;

            // add all DLLs to the list
            if (auto e = entry.path().extension().string(); e == extension)
            {
                dll_names.push_back(entry.path().filename().string());
            }
        }

        // print the names // TODO : Remove
        for (const auto& name : dll_names)
        {
            std::cout << ">> " << name << '\n';
        }

        return dll_names;
    }

}
