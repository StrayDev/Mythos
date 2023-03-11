#include "Module/platform_layer.hpp"

#include "WindowsOS/Window.hpp"

#include "Debug.hpp"


Mythos::platform_layer::platform_layer()
{
	Debug::log_header("Platform Layer : Creating the platform layer");

	window_ = std::make_unique<Window>();
	msg_loop_ = std::make_unique<MessageLoop>();
	msg_hook_ = std::make_unique<MessageHook>();

	msg_hook_->InstallKeyboardHook() = []() { std::cout << "[" << __FILE__ << "] Button Press\n"; };
	/*msg_hook_->InstallMouseHook() = []() { };
	msg_hook_->InstallSystemHook() = []() { };*/

}

Mythos::platform_layer::~platform_layer()
{
	Debug::log_header("Platform Layer : Destroying the platform layer");
}

void Mythos::platform_layer::update()
{
	msg_loop_->Update();
}

void Mythos::platform_layer::render()
{
}
