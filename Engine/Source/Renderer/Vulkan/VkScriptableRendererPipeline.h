#pragma once

class FVkPipelineLayout;
class FVkPipeline;

struct FVkScriptableRendererPipeline
{
    virtual void Initialize(FVkPipelineLayout* PipelineLayout) = 0;
    virtual void Shutdown() = 0;
    virtual void Bind() = 0;
    
    TUniquePtr<FVkPipeline> Pipeline;
};