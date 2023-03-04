#pragma once

// Microsoft
#include <Windows.h>

// STL
#include <iostream>

struct KeyboardEvent 
{
	ULONG code;

}; // temporary move to interface lib


namespace Mythos::Platform::Keyboard
{

	// function pointer
	typedef void (*KeyboardEventCallback)(const KeyboardEvent&);

	// event calback 
	KeyboardEventCallback Callback;

	void SetCallback(const KeyboardEventCallback& callback)
	{
		Callback = callback;
	}

	// invoker 
	void OnEvent(const KeyboardEvent& event)
	{
		if (!Callback) return;
		Callback(event);
	}

	HHOOK Hook; 

	LRESULT CALLBACK Proc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION)
		{
			auto* lData = (KBDLLHOOKSTRUCT*)lParam;

			auto data = KeyboardEvent
			{
				.code = lData->vkCode
			};

			if (wParam == WM_KEYDOWN)
			{
				OnEvent(data);

				std::cout << "Key down event: " << data.code << std::endl;
			}
			else if (wParam == WM_KEYUP)
			{
				auto* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
				std::cout << "Key up event: " << pKeyboard->vkCode << std::endl;
			}
		}

		return CallNextHookEx(Hook, nCode, wParam, lParam);
	}

}
