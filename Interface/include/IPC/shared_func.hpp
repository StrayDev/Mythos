#pragma once

// Microsoft
#include <Windows.h>

// STL
#include <string>
#include <functional>
#include <iostream>

// --
namespace Mythos::IPC
{
	// --

	template <typename T>
	class shared_func;

	template <typename R, typename... Args>
	class shared_func<R(Args...)>
	{
	public:
		// creates shared memory and sets up the function pointe
		explicit shared_func(std::string key);

		// Destructor cleans up the shared memory
		~shared_func();

		// Sets the function to be executed
		void set_function(const std::function<R(Args...)>& function);

		// Executes the function
		R invoke(Args... args) const;

	private:
		std::string key_;
		HANDLE handle_ = nullptr;
		LPVOID map_view_ = nullptr;
		std::function<R(Args...)>* function_ = nullptr;
	};

	template <typename R, typename ... Args>
	shared_func<R(Args...)>::shared_func(std::string key): key_(key)
	{
		// create or open the shared memory
		handle_ = CreateFileMappingA(
			INVALID_HANDLE_VALUE,
			nullptr, PAGE_READWRITE, 0,
			sizeof(std::function<R(Args...)>),
			key_.c_str()
		);

		if (handle_ == nullptr)
		{
			std::cerr << "Error creating shared memory: " << GetLastError() << std::endl;
			return;
		}

		// Map the shared memory into the process's address space
		map_view_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (map_view_ == nullptr)
		{
			std::cerr << "Error mapping shared memory: " << GetLastError() << std::endl;
			CloseHandle(handle_);
			return;
		}

		// Set up the function pointer
		function_ = static_cast<std::function<R(Args...)>*>(map_view_);
	}

	template <typename R, typename ... Args>
	shared_func<R(Args...)>::~shared_func()
	{
		if (!map_view_)
		{
			UnmapViewOfFile(map_view_);
		}
		if (!handle_)
		{
			CloseHandle(handle_);
		}
	}

	template <typename R, typename ... Args>
	void shared_func<R(Args...)>::set_function(const std::function<R(Args...)>& function)
	{
		new (function_) std::function<R(Args...)>(function);
	}

	template <typename R, typename ... Args>
	R shared_func<R(Args...)>::invoke(Args... args) const
	{
		if (*function_)
		{
			return (*function_)(args...);
		}
		return R();
	}
}
