#pragma once

#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"

class FVkBuffer;

struct FVkCullingComputePipeline : public IPipeline2
{
public:
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) override;
    virtual void Shutdown() override;
    virtual void Execute() override;

private:
    TUniquePtr<FVkBuffer> DrawCommandBuffer;
};