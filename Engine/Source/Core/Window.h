#pragma once

class IWindow
{
public:
	virtual bool ShouldClose() const { return false; }
};