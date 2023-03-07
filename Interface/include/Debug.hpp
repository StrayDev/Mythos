#pragma once

// STL
#include <string>
#include <source_location>

// Interface
#include "IPC/shared_func.hpp"
#include "Utility/Constants.hpp"

// --
namespace Mythos::Debug
{
	// --

	static void SetBehaviour(const std::string& key, std::function<void(const std::string&, const std::source_location&)> callback)
	{
#ifdef  _DEBUG
		auto logger = IPC::shared_func<void(const std::string&, const std::source_location&)>(key);
		logger.set_function(callback);
#endif
	}

	static void SetDefaultBehaviour()
	{
#ifdef  _DEBUG
		SetBehaviour(DEBUG_LOG_KEY, [](const std::string& msg, const std::source_location& source)
		{
			std::cout << "[log] " << msg << '\n';
		});

		SetBehaviour(DEBUG_WARN_KEY, [](const std::string& msg, const std::source_location& source)
		{
			std::cout << "[warn] " << msg << '\n';
		});

		SetBehaviour(DEBUG_ERROR_KEY, [](const std::string& msg, const std::source_location& source)
		{
			std::cout << "[error] " << msg << '\n';
		});

		SetBehaviour(DEBUG_ASSERT_KEY, [](const std::string& msg, const std::source_location& source)
		{
			std::cout << "[assert] " << msg << '\n';
		});

		SetBehaviour(DEBUG_EXCEPTION_KEY, [](const std::string& msg, const std::source_location& source)
		{
			std::cout << "[exception] " << msg << '\n';
		});

		std::cout << "[debug] Default log behaviour has been set. \n \n";
#endif
	}

	static void call(const std::string& key, const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		const auto debug = IPC::shared_func<void(const std::string&, const std::source_location&)>(key);
		debug.invoke(msg, source);
#endif
	}

	static void log(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		call(DEBUG_LOG_KEY, msg, source);
#endif
	}

	static void log_header(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		std::cout << '\n';
		call(DEBUG_LOG_KEY, msg, source);
		std::cout << '\n';
#endif
	}

	static void warn(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		call(DEBUG_WARN_KEY, msg, source);
#endif
	}

	static void error(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		call(DEBUG_ERROR_KEY, msg, source);
#endif
	}

	static void assert(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		call(DEBUG_ASSERT_KEY, msg, source);
#endif
	}

	static void exception(const std::string& msg, std::source_location source = std::source_location::current())
	{
#ifdef  _DEBUG
		call(DEBUG_EXCEPTION_KEY, msg, source);
#endif
	}

	static void new_line(const int count = 1)
	{
#ifdef  _DEBUG
		for(int i = 0; i < count; i++ )
		{
			std::cout << '\n';
		}
#endif		
	}
}
