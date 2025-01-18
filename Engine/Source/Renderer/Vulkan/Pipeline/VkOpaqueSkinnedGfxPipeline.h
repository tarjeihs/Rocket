#pragma once

#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"

struct FVkOpaqueSkinnedMeshGfxPipeline : public IPipeline2
{
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) override;
    virtual void Shutdown() override;
    virtual void Execute() override;
};