#pragma once

// STL
#include <functional>
#include <memory>
#include <string>

// Utility
#include "../Utility/Handles.hpp"

// Interface
#include "Module/layer.hpp"

// --
namespace Mythos
{
	class Module
	{
	public:
		mutable MODULE_HANDLE handle;

		int priority;
		int secondary;

		std::string name;
		std::string version;
		std::string description;

		std::string dll_path;
		std::string dll_name;
		std::string dll_date;
		std::string dll_time;

		std::function<std::unique_ptr<layer>()> MakeUniqueLayer;
	};

}