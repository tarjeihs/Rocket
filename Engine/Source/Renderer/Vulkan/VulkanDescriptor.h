#pragma once

#include "EngineTypes.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "VulkanTexture2D.h"

class PVulkanRHI;

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
    void Shutdown();
};


enum class EVkDescriptorType
{
    SSBO,
    SSIO,
    Sampler,
    SamplerImage,
};

struct FVkDescriptorLayout
{
    EVkDescriptorType DescriptorType;
    uint32 DescriptorCount;
};

struct FVkDescriptorSetLayoutCreateInfo
{
    TArray<FVkDescriptorLayout> Descriptors;
};

struct FVkDescriptorSetLayoutInfo
{
    VkDescriptorSetLayout Handle;
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
    FVkDescriptorPool*                  DescriptorPool;
    FVkDescriptorSetLayout*             DescriptorSetLayout;
};

struct FVkDescriptorSetInfo
{
    VkDescriptorSet                     Handle;
    TArray<FVkBuffer**>                 Buffers;
    TArray<FVkTexture2D*>               Textures;
};

class FVkDescriptorSet
{
public:
    FVkDescriptorSetInfo Info;

    void Initialize(FVkDescriptorSetCreateInfo& CreateInfo);
    void Shutdown();

    void WriteBuffer(uint32 Index, FVkBuffer* Buffer);
    void ReadBuffer(uint32 Index, FVkBuffer*& Buffer);

    void WriteTexture2D(uint32 Index, FVkTexture2D* Texture2D);
    void ReadTexture2D(uint32 Index, FVkTexture2D*& Texture2D);

    void Bind(FVkPipelineLayout* PipelineLayout);
};