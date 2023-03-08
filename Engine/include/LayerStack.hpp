#pragma once
#include "Utility/Export.hpp"

// --
namespace Mythos
{
	class layer;
	class Module;
	// --

	class LayerStack
	{
	public:


		std::vector<std::unique_ptr<Module>> modules_;
		std::vector<std::unique_ptr<layer>> layers_;



	};
}
