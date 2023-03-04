#pragma once

#if _WIN64

#define Export __declspec(dllexport)

#else

#define Export

#endif

