#include "WindowsOS/MessageHook.hpp"

// Microsoft
#include <Windows.h>
#include <Windowsx.h>

// STL
#include <iostream>

// Callbacks
#include "WindowsOS/Callbacks/Keyboard.hpp"
#include "WindowsOS/Callbacks/Mouse.hpp"

// --
namespace Mythos::Platform
{

	MessageHook::~MessageHook()
	{
		for (auto hook : hook_list_)
		{
			UnhookWindowsHookEx((HHOOK)hook);
		}
	}

	std::function<void()>& MessageHook::InstallKeyboardHook()
	{
		auto hook = SetWindowsHookEx(WH_KEYBOARD_LL, Keyboard::Proc, nullptr, 0);
		hook_list_.push_back(hook);
		return OnKeyboardProc;
	}

	std::function<void()>& Platform::MessageHook::InstallMouseHook()
	{
		auto hook = SetWindowsHookEx(WH_MOUSE_LL, Mouse::Proc, GetModuleHandle(NULL), 0);
		hook_list_.push_back(hook);
		return OnKeyboardProc;
	}

	std::function<void()>& Platform::MessageHook::InstallSystemHook()
	{
		return OnWindowProc;
	}

}
