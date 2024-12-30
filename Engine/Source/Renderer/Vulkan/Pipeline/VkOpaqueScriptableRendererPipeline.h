#pragma once

#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"

struct FVkOpaqueScriptableRendererPipeline : public FVkScriptableRendererPipeline
{
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) override; 
    virtual void Shutdown() override;
    virtual void Execute() override;
};