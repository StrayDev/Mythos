#pragma once


class IMessageLoop
{
public:
	virtual ~IMessageLoop() = default;

	virtual void Update() = 0;

};

