#pragma once

// Utility
#include "Utility/Handles.hpp"

// takes the derrived type and the handle type
class IWindow
{
public:
	virtual ~IWindow() = default;

	virtual void Create() = 0;
	virtual void Destroy() = 0;

	virtual int GetHeight() = 0;
	virtual int GetWidth() = 0;

	virtual WINDOW_HANDLE GetHandle() = 0;

};
