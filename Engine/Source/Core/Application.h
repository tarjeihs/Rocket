#pragma once

class PApplication
{
public:
	void Run();

	void Exit();
};

// Externally defined per application
PApplication* CreateApplication();