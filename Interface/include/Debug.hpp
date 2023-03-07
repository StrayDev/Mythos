#pragma once

// STL
#include <string>
#include <source_location>

// Interface
#include "IPC/shared_func.hpp"
#include "Utility/Constants.hpp"

namespace Mythos::Debug
{

	static void SetDefaultBehaviour()
	{
		// set the default log behaviour
		auto debug = IPC::shared_func<void(const std::string&)>(DEBUG_KEY);
		debug.set_function([](const std::string& string)
		{
			std::cout << "[ log ] : " << string << '\n';
		});
		debug.invoke("Setting default log behaviour.");
	}


	static void log(const std::string& msg, std::source_location source = std::source_location::current())
	{
		const auto debug = IPC::shared_func<void(const std::string&)>(DEBUG_KEY);
		/*std::cout << "[ line ]" << source.line() << '\n';
		std::cout << "[ file ]" << source.file_name() << '\n';
		std::cout << "[ func ]" << source.function_name() << '\n';*/
		debug.invoke(msg);
	}
}
