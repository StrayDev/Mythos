#include "WindowsOS/MessageLoop.hpp"

// Microsoft
#include <Windows.h>


namespace Mythos::Platform
{
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
