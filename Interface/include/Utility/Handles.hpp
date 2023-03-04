#pragma once

#if _WIN64
#define WIN64_LEAN_AND_MEAN

// Microsoft
#include "Windows.h"

using MODULE_HANDLE = HMODULE;

// Forward declaration

using WINDOW_HANDLE = HWND;

#else

using MODULE_HANDLE = void*;

#endif
