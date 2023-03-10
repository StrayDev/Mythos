#include "WindowsOS/MessageLoop.hpp"

// Microsoft
#include <Windows.h>

// STL
#include <iostream>

#include "Debug.hpp"

// --
namespace Mythos::Platform
{
	MessageLoop::MessageLoop()
	{
		Debug::log(" >> Creating message_loop object");
	}

	MessageLoop::~MessageLoop()
	{
		Debug::log(" >> Destroying message_loop object");
	}

	void MessageLoop::Update()
	{
		/*MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}*/
	}

}
