#pragma once

// --
namespace Mythos
{

	class layer
	{
	public:
		virtual ~layer() = default;

		virtual void update() = 0;
		virtual void render() = 0;
	};
}
