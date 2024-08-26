#pragma once

#include <cstdint>

#include "Utils/Timestep.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class PScene;
class IWindow;
class IRHI;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1920;
static constexpr uint32_t VIEWPORT_HEIGHT = 1080;

class PEngine
{
public:
	STimestep Timestep;
	
	void Start();
	void Run();
	void Stop();

	inline IWindow* GetWindow();
	inline PScene* GetScene();
	inline IRHI* GetRHI();
	
	static PEngine* Get()
	{
		return GEngine;
	}

private:
	IWindow* Window;
	IRHI* RHI;
	PScene* Scene;

	static PEngine* GEngine;
};

inline PScene* PEngine::GetScene()
{
	return PEngine::Get()->Scene;
}

inline IRHI* PEngine::GetRHI()
{
	return RHI;
}

inline IWindow* PEngine::GetWindow()
{
	return PEngine::Get()->Window;
}