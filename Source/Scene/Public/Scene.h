#pragma once 

#include "Core/Public/Camera.h"
#include "Core/Public/Engine.h"
#include "Scene/Public/Component.h"
#include "Scene/Public/Registry.h"

class PScene
{
public:
	void Init()
	{
		Camera = new PCamera();
		Registry = new PRegistry();

#if RK_DEBUG
//		GOverlay->OnRender.Bind(this, &PScene::OnImGuiRender);
#endif
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

	void OnImGuiRender();

private:
	PCamera* Camera;

	PRegistry* Registry;
};

inline static PScene* GetScene()
{
	return GetEngine()->GetScene();
}
