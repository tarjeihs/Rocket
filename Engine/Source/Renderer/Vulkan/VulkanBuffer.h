#pragma once

#include "EnginePCH.h"
#include "EngineTypes.h"
#include "Renderer/Common/Buffer.h"

struct FVkBufferInfo
{
    VkBuffer Handle;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;
};

class FVkBuffer : public IBuffer
{
public:
    FVkBufferInfo Info;

    void Initialize(const FBufferCreateInfo& CreateInfo) override;
    void Shutdown() override;
    void Submit(const void* Data, size_t Size, size_t Offset = 0) override;
};