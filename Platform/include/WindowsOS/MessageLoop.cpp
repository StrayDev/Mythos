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

        bool move;

	void MessageLoop::Update()
    {
        MSG msg;
        int maxMessages = 10;
        int numMessages = 0;

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) && numMessages < maxMessages)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            numMessages++;
        }
    }

}
