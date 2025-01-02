#pragma once

#include "EnginePCH.h"
#include "EngineTypes.h"

enum class EVkBufferType
{
    None = 0,
    StorageBuffer,
    StorageImage,
    Sampler,
    SamplerImage
};

struct FVkBufferCreateInfo
{
    VkBufferUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsageFlags;
    SizeType Size;
};

struct FVkBufferInfo
{
    VkBuffer Handle;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;
};

class FVkBuffer
{
public:
    FVkBufferInfo Info;

    void Initialize(FVkBufferCreateInfo& CreateInfo);
    void Submit(const void* Data, size_t Size, size_t Offset = 0);
    void Free();
};