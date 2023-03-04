#pragma once

// STL
#include <functional>

// --
namespace Mythos::Platform
{

	class MessageHook
	{
	public:
		MessageHook() = default;
		~MessageHook();

		std::function<void()>& InstallKeyboardHook();
		std::function<void()>& InstallMouseHook();
		std::function<void()>& InstallSystemHook();

	private:
		std::vector<void*> hook_list_;

		std::function<void()> OnKeyboardProc = nullptr;
		std::function<void()> OnMouseProc = nullptr;
		std::function<void()> OnWindowProc = nullptr;

	};

}

//template<HookType type>
//auto InstallHook() -> decltype(auto)
//{
//	HHOOK hook;
//
//	auto SetHook = [this](int idHook, HOOKPROC proc)
//	{
//		hook = SetWindowsHookEx(idHook, proc, GetModuleHandle(NULL), 0);
//		hook_list_.push_back(hook);
//	};
//
//	switch (type)
//	{
//		case HookType::Keyboard;
//		{
//			SetHook(WH_KEYBOARD_LL, Keyboard::Proc);
//			return OnKeyProc;
//			break;
//		}
//
//		case HookType::Mouse;
//		{
//			SetHook(WH_Mouse_LL, Mouse::Proc);
//			return OnMouseProc;
//			break;
//		}
//
//		default;
//		return nullptr;
//	}
//}