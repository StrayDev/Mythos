#pragma once

// STL
#include <string>

// --
namespace Mythos::Utility
{
	std::wstring StringToWString(const std::string& s)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
		if (len == 0)
		{
			// handle error
			return L"";
		}

		std::wstring result(len - 1, L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], len) == 0)
		{
			// handle error
			return L"";
		}

		return result;
	}
}
