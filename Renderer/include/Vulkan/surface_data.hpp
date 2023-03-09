#pragma once

#ifndef _WINDEF_

typedef void* HANDLE;

struct HWND__;
typedef HWND__* HWND;

struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

struct HMONITOR__;
typedef HMONITOR__* HMONITOR;


typedef const wchar_t* LPCWSTR;
typedef unsigned long  DWORD;

struct _SECURITY_ATTRIBUTES;
typedef _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;

#endif

// Vulkan
#include <Vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>


// --
namespace Mythos::vulkan
{
	// --

	struct surface_data
	{
		VkSurfaceKHR handle = VK_NULL_HANDLE;

		VkWin32SurfaceCreateInfoKHR create_info
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		};

		
	};
}
