#include "Application.hpp"

// STL
#include <iostream>
#include <vector>

// Utility
#include "Utility/ModuleUtility.hpp"


// --
Mythos::Application::Application()
{
	// try populate the modules
	if(!Utility::TryLoadModules(modules_))
	{
		std::cout << "Utility::TryLoadModules() : failed to load the modules" << '\n' << __LINE__ << " : " << __FILE__ << '\n';

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
		std::cout << "Utility::TryCreateLayers() : failed to create the layers" << '\n' << __LINE__ << " : " << __FILE__ << '\n';
		return;
	}

	// enable run loop 
	is_running = true;
}

Mythos::Application::~Application()
{
	std::cout << "Application Destroyed\n";
}

void Mythos::Application::Run()
{
	while (true)
	{
		for (const auto& layer : layers_)
		{
			layer->Update();
			layer->Render();
		}
	}
}
