#pragma once

#include <cstdint>

class PScene;
class IWindow;
class IRenderer;

static constexpr char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1920;
static constexpr uint32_t VIEWPORT_HEIGHT = 1080;

class PEngine
{
public:
	void Start();
	void Run();
	void Stop();

	inline PScene* GetScene() const;
	inline IWindow* GetWindow() const;
	inline IRenderer* GetRenderer() const;

private:
	PScene* Scene;
	IWindow* Window;
	IRenderer* Renderer;

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