#pragma once

// --
namespace Mythos
{
	// Priority Ranges for Modules and Layers
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
