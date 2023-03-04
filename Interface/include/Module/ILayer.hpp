#pragma once

// --
namespace Mythos
{

	class ILayer
	{
	public:
		virtual ~ILayer() = default;

		virtual void Update() = 0;
		virtual void Render() = 0;
	};
}
