#pragma once

// engine include file
#include "Mythos.hpp"


class Sandbox : public Mythos::application
{
public:
	Sandbox() = default;
	~Sandbox() final = default;


private:


};


// this defines allows us to inject the derived application 
inline std::unique_ptr<Mythos::application> Mythos::create_application()
{
	return std::make_unique<Sandbox>();
}
