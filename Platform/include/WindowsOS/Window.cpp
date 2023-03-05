#include "WindowsOS/Window.hpp"

// Microsoft
#include <Windows.h>
#include <iostream>

//--
namespace Mythos::Platform
{

	Window::Window()
	{
		std::cout << " >> Creating Window Object\n";
		Create();
	}

	Window::~Window()
	{
		Destroy();
	}

	void Window::Create()
	{
		// Create the window class
		WNDCLASSEX wcex =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = DefWindowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = GetModuleHandle(NULL),
			.hIcon = LoadIcon(NULL, IDI_APPLICATION),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
			.lpszMenuName = NULL,
			.lpszClassName = L"MyWindowClass",
			.hIconSm = LoadIcon(NULL, IDI_APPLICATION),
		};

		// Register the class
		if (!RegisterClassEx(&wcex))
		{
			// Handle the error
			std::cout << "failed to register\n";
			return;
		}

		// Create the window
		handle = CreateWindowEx(
			0,
			L"MyWindowClass",
			L"My Window",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);

		if (!handle)
		{
			// Handle the error
			std::cout << "failed to get handle\n";
			return;
		}

		// Display the window
		ShowWindow(handle, SW_SHOW);
		UpdateWindow(handle);
	}

	void Window::Destroy()
	{
		if (handle)
		{
			DestroyWindow(handle);
		}
	}

	int Window::GetHeight()
	{
		return 0;
	}

	int Window::GetWidth()
	{
		return 0;
	}

	HWND Window::GetHandle()
	{
		return handle;
	}

}
