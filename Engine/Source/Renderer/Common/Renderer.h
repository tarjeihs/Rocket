#pragma once

#include "Core/Class.h"
#include "Renderer/Common/Buffer.h"
#include "Renderer/Common/Image.h"
#include "Renderer/Common/Pipeline.h"
#include "Types/Map.h"
#include "Types/String.h"

struct FRendererContext
{
    TArray<TPair<FString, FBufferCreateInfo>>    BufferCreateInfos;
    TArray<TPair<FString, FImageCreateInfo>>     ImageCreateInfos;
    TArray<TPair<FString, FPipelineCreateInfo>>  PipelineCreateInfos;
};

class IRenderer 
{
public:
    virtual ~IRenderer() = default;

    virtual void CreateRendererContext(FRendererContext& Ctx) = 0;
};

extern inline IRenderer* GRenderer = nullptr;