#include "Module/Layer.hpp"

#include <iostream>

#include "WindowsOS/Window.hpp"


Mythos::Layer::Layer()
{
	std::cout << "\nCreating Platform Layer :\n\n";

	window_ = std::make_unique<Window>();
	msg_loop_ = std::make_unique<MessageLoop>();
	msg_hook_ = std::make_unique<MessageHook>();

	msg_hook_->InstallKeyboardHook() = []() { std::cout << "[" << __FILE__ << "] Button Press\n";  };
	/*msg_hook_->InstallMouseHook() = []() { };
	msg_hook_->InstallSystemHook() = []() { };*/
}

Mythos::Layer::~Layer()
{

}

void Mythos::Layer::Update()
{
	msg_loop_->Update();
}

void Mythos::Layer::Render()
{

}
