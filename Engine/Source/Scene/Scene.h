#pragma once 

#include "Core/Camera.h"
#include "Core/Engine.h"
#include "Scene/Registry.h"

struct SMeshComponent;
struct STransformComponent;

class PScene
{
public:
	void Init()
	{
		Camera = new PCamera();
		Registry = new PRegistry();
	}

	void Update()
	{

	}

	void Cleanup() 
	{
		delete Camera;
		delete Registry;
	}

	[[nodiscard]] PCamera* GetCamera() const
	{
		return Camera;
	}

	[[nodiscard]] PRegistry* GetRegistry() const
	{
		return Registry;
	}

private:
	PCamera* Camera;

	PRegistry* Registry;
};

inline static PScene* GetScene()
{
	return GetEngine()->GetScene();
}
