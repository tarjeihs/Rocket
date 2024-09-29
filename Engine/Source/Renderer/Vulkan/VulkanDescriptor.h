#pragma once

#include <span>
#include <vector>

#include "Renderer/Vulkan/VulkanBuffer.h"

class PVulkanRHI;
struct PVulkanBuffer;
struct VkDescriptorPool_T;
struct VkDescriptorSet_T;
struct VkDescriptorSetLayout_T;
struct VkDescriptorSetLayoutBinding;
enum VkDescriptorType;

typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef uint64_t VkDeviceSize;

struct SVulkanDescriptorPoolRatio
{
    VkDescriptorType Type;
    float Ratio;
};

class PVulkanDescriptorPool
{
public:
    void CreatePool(uint32_t MaxSets, std::span<SVulkanDescriptorPoolRatio> PoolRatios, uint32_t Flags = 0);
    void DestroyPool();
    void ResetPool();

    VkDescriptorPool GetVkDescriptorPool() const;

private:
    VkDescriptorPool Pool;
};

class PVulkanDescriptorSetLayout
{
public:
    enum class EDescriptorSetLayoutType { Storage, Uniform };

    void CreateDescriptorSetLayout(std::vector<EDescriptorSetLayoutType> LayoutTypes);
    void FreeDescriptorSetLayout();

    VkDescriptorSetLayout GetVkDescriptorSetLayout() const;

protected:
    VkDescriptorSetLayoutBinding CreateStorageBufferBinding(uint32_t Binding);
    VkDescriptorSetLayoutBinding CreateUniformBufferBinding(uint32_t Binding);

private:
    VkDescriptorSetLayout DescriptorSetLayout;
};

class PVulkanDescriptorSet
{
public:
    void CreateDescriptorSet(PVulkanDescriptorSetLayout* DescriptorSetLayout);
    void FreeDescriptorSet();

    void UseDescriptorStorageBuffer(PVulkanBuffer* Buffer, VkDeviceSize Offset, VkDeviceSize Range, uint32_t Binding);
    void UseDescriptorUniformBuffer(PVulkanBuffer* Buffer, VkDeviceSize Offset, VkDeviceSize Range, uint32_t Binding);

    VkDescriptorSet GetVkDescriptorSet() const;

private:
	VkDescriptorSet DescriptorSet;
};