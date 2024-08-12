#pragma once

#include "Core/Engine.h"

class IRenderer
{
public:
	virtual void Initialize() = 0;
	virtual void DestroyRenderer() = 0;

	virtual void Draw() = 0;
};

template<typename TRenderer>
inline static TRenderer* GetRenderer()
{
	return static_cast<TRenderer*>(PEngine::Get()->GetRenderer());
}