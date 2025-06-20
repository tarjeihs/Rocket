#pragma once

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class PScene;
class IWindow;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

class CEngine
{
public:
	void Start();
	void Run();
	void Stop();

	inline PScene* GetScene();
	inline IWindow* GetWindow();
	
	inline friend CEngine* GetEngine();
	
private:
	PScene* Scene;
	IWindow* Window;

	static CEngine* GEngine;
};

inline PScene* CEngine::GetScene()
{
	return Scene;
}

inline IWindow* CEngine::GetWindow()
{
	return Window;
}

inline CEngine* GetEngine()
{
	return CEngine::GEngine;
}
