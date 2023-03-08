#pragma once

// Interface
#include "Module/layer.hpp"

// --
namespace Mythos
{
	// --

	class event_layer : public layer
	{
	public:
		event_layer();
		~event_layer() override;
		void update() override;
		void render() override;

	private:

	};
}
