#include "WindowsOS/MessageLoop.hpp"

// Microsoft
#include <Windows.h>

// STL
#include <iostream>

// --
namespace Mythos::Platform
{
	MessageLoop::MessageLoop()
	{
		std::cout << " >> Creating MessageLoop Object\n";
	}

	void MessageLoop::Update()
	{
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

}
