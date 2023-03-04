#pragma once

// STL
#include <memory>
#include <vector>

// Interface
#include "Module/Module.hpp"

// --
namespace Mythos
{

	class Application
	{
	public:
		Application();
		virtual ~Application();
		
		void Run();


	private:
		std::vector<std::unique_ptr<Module>> modules_;
		std::vector<std::unique_ptr<ILayer>> layers_;

		bool is_running;

	};

	extern std::unique_ptr<Application> CreateApplication();
}
