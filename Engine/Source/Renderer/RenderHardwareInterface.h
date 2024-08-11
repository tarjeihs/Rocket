#pragma once

class IRenderHardwareInterface
{
public:
	virtual ~IRenderHardwareInterface() = default;

	virtual void CreateRHI() = 0;
	virtual void DestroyRHI() = 0;
	virtual void Render() = 0;
};