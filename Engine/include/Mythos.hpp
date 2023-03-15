#pragma once

// Base Application class
#include "Application.hpp"

// Entry point for the application
#include "Entry.hpp"

// To be defined with the derived class
extern std::unique_ptr<Mythos::application> Mythos::create_application();


