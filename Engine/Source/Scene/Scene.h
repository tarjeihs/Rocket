#pragma once 

#include "Component.h"
#include "Core/Camera.h"
#include "Core/Engine.h"
#include "Scene/Registry.h"
#include "Asset/AssetManager.h"

struct SMeshComponent;
struct STransformComponent;

class PScene
{
public:
	void Init()
	{
		Camera = new PCamera();
		Registry = new PRegistry();
		AssetManager = new PAssetManager();
	}

	void Update()
	{

	}

	void Cleanup() 
	{
		delete Camera;
		delete Registry;
		delete AssetManager;
	}

	[[nodiscard]] PCamera* GetCamera() const
	{
		return Camera;
	}

	[[nodiscard]] PRegistry* GetRegistry() const
	{
		return Registry;
	}

	[[nodiscard]] PAssetManager* GetAssetManager() const
	{
		return AssetManager;
	}

private:
	PCamera* Camera;

	PRegistry* Registry;

	PAssetManager* AssetManager;
};

inline static PScene* GetScene()
{
	return GetEngine()->GetScene();
}
