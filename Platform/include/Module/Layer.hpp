#pragma once

// STL
#include <memory>

// Interface
#include "Module/ILayer.hpp"

// Components
#include "WindowsOS/MessageHook.hpp"
#include "WindowsOS/MessageLoop.hpp"
#include "WindowsOS/Window.hpp"

// --
namespace Mythos
{
	// --
	using namespace Platform;

	class Layer : public ILayer
	{
	public:
		Layer();
		~Layer() override;

		void Update() override;
		void Render() override;
		
	private:
		std::unique_ptr<Window> window_;
		std::unique_ptr<MessageLoop> msg_loop_;
		std::unique_ptr<MessageHook> msg_hook_;
	};
}
