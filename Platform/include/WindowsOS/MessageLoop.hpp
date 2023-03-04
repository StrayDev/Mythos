#pragma once

// Interface
#include "Interface/IMessageLoop.hpp"


namespace Mythos::Platform
{

	class MessageLoop
	{
	public:
		MessageLoop() = default;
		~MessageLoop() = default;

		void Update();

	};

}