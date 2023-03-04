#pragma once

// Interface
#include "Interface/IWindow.hpp"


// --
namespace Mythos::Platform
{

	class Window
	{
	public:
		Window();
		~Window();

		void Create();
		void Destroy();

		int GetHeight();
		int GetWidth();

		WINDOW_HANDLE GetHandle();

	private:
		WINDOW_HANDLE handle;
	};

}