#pragma once

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class PScene;
class IWindow;
class IRHI;

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
	inline IRHI* GetRHI();
	
	inline friend CEngine* GetEngine();
	
private:
	PScene* Scene;
	IWindow* Window;
	IRHI* Renderer;

	static CEngine* GEngine;
};

inline PScene* CEngine::GetScene()
{
	return Scene;
}

inline IRHI* CEngine::GetRHI()
{
	return Renderer;
}

inline IWindow* CEngine::GetWindow()
{
	return Window;
}

inline CEngine* GetEngine()
{
	return CEngine::GEngine;
}
