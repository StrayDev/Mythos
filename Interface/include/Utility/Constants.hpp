#pragma once

// --
namespace Mythos
{
	// shared memory keys
	const std::string DEBUG_LOG_KEY = "DEBUG_LOG_KEY";
	const std::string DEBUG_WARN_KEY = "DEBUG_WARN_KEY";
	const std::string DEBUG_ERROR_KEY = "DEBUG_ERROR_KEY";
	const std::string DEBUG_ASSERT_KEY = "DEBUG_ASSERT_KEY";
	const std::string DEBUG_EXCEPTION_KEY = "DEBUG_EXCEPTION_KEY";


	// priority ranges for modules and layers
	constexpr int PLATFORM = 100;
	constexpr int DEBUG = 200;
	constexpr int INPUT = 300;
	constexpr int RESOURCE = 400;
	constexpr int PHYSICS = 500;
	constexpr int RENDER = 600;
	constexpr int EVENT = 700;
	constexpr int JOB = 800;
	constexpr int PLUGIN = 1001;
	constexpr int PLUGIN_MAX_RANGE = 9999;
}
