#pragma once

#include <cstdint>

class PScene;
class IWindow;
class IRenderHardwareInterface;

static constexpr char* VIEWPORT_NAME = "Rocket Engine";
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
	inline IRenderHardwareInterface* GetRenderHardwareInterface() const;

private:
	PScene* Scene;
	IWindow* Window;
	IRenderHardwareInterface* RenderHardwareInterface;

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

inline IRenderHardwareInterface* PEngine::GetRenderHardwareInterface() const
{
	return RenderHardwareInterface;
}