#pragma once

#include "Renderer/Vulkan/VkRenderer.h"

class FVkForwardRenderer : public FVkRenderer
{
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Bind() override;
    virtual void Resize() override;
    virtual void BindImGui() override;

    void OnSubmitGlobalBuffer();
    void OnSubmitCameraBuffer();
    void OnSubmitMaterialBuffer();
    void OnSubmitInstanceBuffer();
};