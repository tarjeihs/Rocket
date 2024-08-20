#pragma once 

#include "Camera.h"
#include "Engine.h"

class PScene
{
public:
	void Init()
	{
		ActiveCamera = new PCamera();
	}
	
	PCamera* ActiveCamera;
};

inline static PScene* GetScene()
{
	return PEngine::Get()->GetScene();
}