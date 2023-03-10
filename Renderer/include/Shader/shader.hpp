#pragma once

// STL
#include <fstream>
#include <filesystem>
#include <vector>

// Mythos
#include "Debug.hpp"

// --
namespace Mythos::shader
{
	// --

	static std::vector<char> read_file(const std::string& file_name)
	{
		auto file = std::ifstream(file_name, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			const auto path = std::filesystem::current_path().generic_string();
			Debug::error("Shader failed to open file : " + file_name + ", at : " + path);
			return {};
		}

		const auto file_size = static_cast<size_t>(file.tellg());
		auto buffer = std::vector<char>(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		file.close();

		Debug::log("Shader file loaded : " + file_name);
		return buffer;
	}

}
