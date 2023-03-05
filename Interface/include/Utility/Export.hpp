#pragma once


#ifdef _WIN64 
	#ifdef _WINDLL
		#ifdef MYTHOS_ENGINE
			#define ENGINE_API __declspec(dllexport)
			#define MODULE_API __declspec(dllimport)
		#else
			#define ENGINE_API __declspec(dllimport)
			#define MODULE_API __declspec(dllexport)
		#endif
	#else
		#define ENGINE_API __declspec(dllimport)
		#define MODULE_API __declspec(dllimport)
	#endif
#else
	#error Mythos Only Supports Windows 64.
#endif
