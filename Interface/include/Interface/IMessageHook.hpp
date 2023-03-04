#pragma once

// Interface
#include "IMakeUnique.hpp"

// STL
#include <functional>

enum class HookType
{
	Window,
	Mouse,
	Keyboard,
};

template <typename T>
class IMessageHook : public IMakeUnique<T>
{
public:
	virtual ~IMessageHook() = default;

	virtual std::function<void()>& InstallKeyboardHook() = 0;
	virtual std::function<void()>& InstallMouseHook() = 0;
	virtual std::function<void()>& InstallSystemHook() = 0;

};
