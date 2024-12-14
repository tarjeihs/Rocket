#pragma once

#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Types/SharedPtr.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

class PVulkanRHI;
class PVulkanFrame;
struct PVulkanBuffer;

struct FVkDescriptorPoolRatio
{
    VkDescriptorType Type;
    float Ratio;
};

struct FVkDescriptorPoolCreateInfo
{
    TArray<FVkDescriptorPoolRatio> PoolRatios;
    uint32_t MaxSetCount;
    uint32_t Flags;
};

struct FVkDescriptorPoolInfo
{
    VkDescriptorPool DescriptorPool;
};

class FVkDescriptorPool
{
public:
    FVkDescriptorPoolInfo Info;

    void Initialize(FVkDescriptorPoolCreateInfo& CreateInfo);
    void Destroy();
};

enum class EVkDescriptorType
{
    Storage,
    StorageImage,
    Sampler,
    SamplerImage,
};

struct FVkDescriptor
{
    EVkDescriptorType DescriptorType;
    uint32 DescriptorCount;
};

struct FVkDescriptorSetLayoutCreateInfo
{
    TArray<FVkDescriptor> Descriptors;
};

struct FVkDescriptorSetLayoutInfo
{
    VkDescriptorSetLayout DescriptorSetLayout;
};

class FVkDescriptorSetLayout
{
public:
    FVkDescriptorSetLayoutInfo Info;

    void Initialize(FVkDescriptorSetLayoutCreateInfo& CreateInfo);
    void Destroy();
};

struct FVkDescriptorSetCreateInfo
{
    TSharedPtr<FVkDescriptorPool> DescriptorPool;
    TSharedPtr<FVkDescriptorSetLayout> DescriptorSetLayout;
};

struct FVkDescriptorSetInfo
{
    VkDescriptorSet Handle;
};

class FVkDescriptorSet
{
public:
    FVkDescriptorSetInfo Info;

    void Initialize(FVkDescriptorSetCreateInfo& CreateInfo);
    void Destroy();

    void AttachBuffer(uint32 Index, const TSharedPtr<FVkBuffer>& Buffer);
    void AddSampler();
    void AddImage();

    void Bind(const TSharedPtr<FVkPipelineLayout>& PipelineLayout);
};