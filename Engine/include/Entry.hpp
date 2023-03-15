#pragma once

#include "Application.hpp"

// --
int main()
{
	const auto app = Mythos::create_application();

	app->run(); 

	app->shutdown();

	return 0;
}
