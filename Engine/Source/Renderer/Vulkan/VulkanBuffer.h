#pragma once

#include "EnginePCH.h"
#include "EngineTypes.h"
#include "Renderer/Common/Buffer.h"

struct VkBuffer_T;
struct VmaAllocation_T;
struct VmaAllocationInfo;

typedef VkBuffer_T* VkBuffer;
typedef VmaAllocation_T* VmaAllocation;
typedef uint32_t VkBufferUsageFlags;

enum VmaMemoryUsage;

class PVulkanBuffer : public IBuffer
{
public:
    PVulkanBuffer(VkBufferUsageFlags InUsageFlags, VmaMemoryUsage InMemoryUsageFlags)
        : UsageFlags(InUsageFlags), MemoryUsageFlags(InMemoryUsageFlags)
    {
    }

    virtual void Allocate(size_t Size) override;
    virtual void Free() override;
    virtual void Submit(const void* Data, size_t Size, size_t Offset = 0) override;

    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;

    VkBufferUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsageFlags;
};

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
    EVkBufferType BufferType;
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
};