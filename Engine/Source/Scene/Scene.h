#pragma once 

#include "Core/Camera.h"
#include "Core/Engine.h"
#include "Scene/Component.h"
#include "Scene/Registry.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

struct FMeshComponent;
struct FTransformComponent;

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
