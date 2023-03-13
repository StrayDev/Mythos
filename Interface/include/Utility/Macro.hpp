#pragma once

#include <cstring>

#if _WIN64

#define WIN64_LEAN_AND_MEAN

#define PLATFORM_WINDOWS _WIN64

inline const char* ToName(const char* file_path)
{
	const char* filename = file_path;
	const char* last_separator = strrchr(filename, '\\');
	if (last_separator == nullptr) last_separator = strrchr(filename, '/');
	if (last_separator == nullptr) last_separator = filename - 1;
	return last_separator + 1;
}

#define FILE_DATE __DATE__

#define FILE_TIME __TIME__

#define FILE_NAME ToName(__FILE__)

#define FILE_PATH __FILE__

#else

#define FILE_NAME ""

#define FILE_PATH ""

#endif
