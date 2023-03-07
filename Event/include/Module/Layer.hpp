#pragma once

// Interface
#include "Module/ILayer.hpp"

// --
namespace Mythos
{
	// --

	class Layer : public ILayer
	{
	public:
		Layer();
		~Layer() override;
		void Update() override;
		void Render() override;

	private:

	};
}
