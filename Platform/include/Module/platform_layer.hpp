#pragma once

// STL
#include <memory>

// Interface
#include "Module/layer.hpp"

// Components
#include "WindowsOS/MessageHook.hpp"
#include "WindowsOS/MessageLoop.hpp"
#include "WindowsOS/Window.hpp"

// --
namespace Mythos
{
	// --
	using namespace Platform;

	class platform_layer : public layer
	{
	public:
		platform_layer();
		~platform_layer() override;

		void update() override;
		void render() override;
		
	private:
		std::unique_ptr<Window> window_;
		std::unique_ptr<MessageLoop> msg_loop_;
		std::unique_ptr<MessageHook> msg_hook_;
	};
}
