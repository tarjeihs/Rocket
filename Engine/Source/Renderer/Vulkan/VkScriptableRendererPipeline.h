#pragma once

#include "Renderer/Common/Shader.h"

class FVkPipelineLayout;
class FVkPipeline;

struct IPipeline2
{
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) = 0;
    virtual void Shutdown() = 0;
    virtual void Execute() = 0;

    TUniquePtr<FVkPipeline> Pipeline;
};