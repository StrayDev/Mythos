#pragma once

// --
namespace Mythos::Platform
{

	class MessageLoop
	{
	public:
		MessageLoop();
		~MessageLoop() = default;

		void Update();

	};

}