#pragma once

#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"

class FVkPostProcessComputePipeline : public IPipeline2
{
public:
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) override;
    virtual void Shutdown() override;
    virtual void Execute() override;
};