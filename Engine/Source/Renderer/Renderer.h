#pragma once

#include "Core/Engine.h"

class IRenderer
{
public:
	virtual void Initialize() = 0;
	virtual void DestroyRenderer() = 0;
	virtual void Draw() = 0;
	virtual void Resize() = 0;
};

inline static IRenderer* GetRenderer()
{
	return PEngine::Get()->GetRenderer();
}