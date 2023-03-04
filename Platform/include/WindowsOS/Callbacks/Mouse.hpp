#pragma once

// Microsoft
#include <Windows.h>

// STL
#include <iostream>

namespace Mythos::Platform::Mouse
{
	HHOOK Hook;

	LRESULT CALLBACK Proc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode != HC_ACTION)
		{
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(GetForegroundWindow(), &p);

		switch (wParam)
		{
		case WM_MOUSEMOVE:
			std::cout << "Move" << p.x << "," << p.y << std::endl;
			break;

		case WM_LBUTTONDOWN:
			std::cout << "Left Down" << std::endl;
			break;

		case WM_LBUTTONUP:
			std::cout << "Left Down" << std::endl;
			break;

		case WM_RBUTTONDOWN:
			std::cout << "Right Down" << std::endl;
			break;

		case WM_RBUTTONUP:
			std::cout << "Right Down" << std::endl;
			break;

		case WM_MOUSEWHEEL:
			std::cout << "MouseWheel Verticle" << std::endl;
			break;

		case WM_MOUSEHWHEEL:
			std::cout << "MouseWheel Horizontal" << std::endl;
			break;

		case WM_VSCROLL:
			std::cout << "Scroll Verticle" << std::endl;
			break;

		case WM_HSCROLL:
			std::cout << "Scroll Horizontal" << std::endl;
			break;
		}

		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}

}