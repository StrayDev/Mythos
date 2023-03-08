#pragma once

// STL
#include <memory>
#include <vector>

// Interface
#include "LayerStack.hpp"
#include "Module/Module.hpp"
#include "Utility/Export.hpp"

// --
namespace Mythos
{
	// --

	class ENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();
		
		void Run();
		void shutdown();
	private:
		bool is_running_;

	};

	std::vector<std::unique_ptr<Module, std::default_delete<Module>>> modules_;
	std::vector<std::unique_ptr<layer>> layers_;

	extern std::unique_ptr<Application> CreateApplication();
}
