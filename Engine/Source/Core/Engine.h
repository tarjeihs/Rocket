#pragma once

#include <cstdint>

#include "Utils/Timestep.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class PScene;
class IWindow;
class IRenderer;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

class PEngine
{
public:
	void Start();
	void Run();
	void Stop();

	inline PScene* GetScene() const;
	inline IWindow* GetWindow() const;
	inline IRenderer* GetRenderer() const;

	static PEngine* Get()
	{
		return GEngine;
	}

private:
	PScene* Scene;
	IWindow* Window;
	IRenderer* Renderer;

	STimestep Timestep;

	static PEngine* GEngine;
};

inline PScene* PEngine::GetScene() const
{
	return Scene;
}

inline IWindow* PEngine::GetWindow() const
{
	return Window;
}

inline IRenderer* PEngine::GetRenderer() const
{
	return Renderer;
}