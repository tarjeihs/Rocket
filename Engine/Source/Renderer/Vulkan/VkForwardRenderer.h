#pragma once

#include "Renderer/Vulkan/VkRenderer.h"

class FVkForwardRenderer : public FVkRenderer
{
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Bind() override;
};