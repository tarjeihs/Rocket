#pragma once 

#include "Core/Camera.h"
#include "Core/Engine.h"
#include "Scene/Registry.h"

class PScene
{
public:
	void Init()
	{
		Camera = new PCamera();

		Registry = new PRegistry();
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
	return PEngine::Get()->GetScene();
}
