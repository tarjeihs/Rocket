#pragma once

#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"

class FVkOverlayScriptableRendererPipeline : public FVkScriptableRendererPipeline
{
public:
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) override;
    virtual void Shutdown() override;
    virtual void Execute() override;
};