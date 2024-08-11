#pragma once

class PApplication
{
public:
	void Run();

	void Exit();

	class PEngine* Engine;
};

// Externally defined per application
PApplication* CreateApplication();