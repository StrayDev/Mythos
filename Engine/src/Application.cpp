#include "Application.hpp"

// STL
#include <iostream>

// Utility
#include "Utility/ModuleUtility.hpp"

#include "Debug.hpp"

// --
Mythos::Application::Application()
{
	// set the default log behaviour
	Debug::SetDefaultBehaviour();


	// try populate the modules
	if(!Utility::TryLoadModules(modules_))
	{
		Debug::error("Utility::TryLoadModules() : failed to load modules");

		// release modules if partially loaded
		for(const auto& m : modules_)
		{
			Utility::UnloadModule(m);
		}
		return;
	}

	// try populate the layers
	if(!Utility::TryCreateLayers(modules_, layers_))
	{
		Debug::error("Utility::TryCreateLayers() : failed to create layers");
		return;
	}

	// enable run loop 
	is_running_ = true;
}

Mythos::Application::~Application()
{
	Debug::log("Application Destroyed");

	// destroy layers lowest priority first
	for(auto i = layers_.size() ; !layers_.empty(); --i)
	{
		layers_.pop_back();
	}
}

void Mythos::Application::Run()
{
	while (is_running_)
	{
		for (auto& layer : layers_)
		{
			layer->update();
			layer->render();
		}
	}
}

void Mythos::Application::shutdown()
{
	// destroy the layers first
	layers_.clear();

	// TODO : unload the modules

	modules_.clear();
}
