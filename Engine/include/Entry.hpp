#pragma once
#include <iostream>

// --
int main()
{
	const auto app = Mythos::CreateApplication();

	app->Run(); 

	app->shutdown();

	auto temp = std::string();
	std::getline(std::cin, temp);

	return 0;
}
