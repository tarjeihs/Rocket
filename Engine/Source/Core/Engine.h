#pragma once

#include <cstdint>

#include "Utils/Timestep.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class PScene;
class IWindow;
class IRHI;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1440;
static constexpr uint32_t VIEWPORT_HEIGHT = 840;

class PEngine
{
public:
	STimestep Timestep;
	
	void Start();
	void Run();
	void Stop();

	inline PScene* GetScene();
	inline IWindow* GetWindow();
	inline IRHI* GetRHI();
	
	inline friend PEngine* GetEngine();
	
private:
	PScene* Scene;
	IWindow* Window;
	IRHI* RHI;

	static PEngine* GEngine;
};

inline PScene* PEngine::GetScene()
{
	return Scene;
}

inline IRHI* PEngine::GetRHI()
{
	return RHI;
}

inline IWindow* PEngine::GetWindow()
{
	return Window;
}

inline PEngine* GetEngine() 
{
	return PEngine::GEngine;
}