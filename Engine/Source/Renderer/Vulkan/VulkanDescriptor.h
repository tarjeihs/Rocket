#pragma once

#include <span>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanSampler.h"
#include "Renderer/Vulkan/VulkanTexture2D.h"

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

enum class EDescriptorSetBindingType : uint8_t
{
    Storage, Uniform
};

enum class EDescriptorSetBindingFlag : uint8_t
{
    Vertex, Fragment
};

struct SDescriptorSetBindingMemberLayout
{
    std::string Name;
    size_t Size;
    size_t Offset;
};

struct SDescriptorSetBindingLayout
{
    std::string Name;
    uint32_t Binding;
    size_t Size;

    EDescriptorSetBindingType Type;
    EDescriptorSetBindingFlag Flag;

    std::vector<SDescriptorSetBindingMemberLayout> Members;
};

struct SDescriptorSetBinding
{
    SDescriptorSetBindingLayout* Layout;
    void* Data;
};

class PVulkanDescriptorSetLayout
{
public:
    void CreateDescriptorSetLayout(const std::vector<SDescriptorSetBindingLayout>& Data);
    void DestroyDescriptorSetLayout();

    VkDescriptorSetLayout GetVkDescriptorSetLayout() const;
    
    std::span<SDescriptorSetBindingLayout> GetBindings();
    std::span<const SDescriptorSetBindingLayout> GetBindings() const;

private:
    std::vector<SDescriptorSetBindingLayout> Bindings;

    VkDescriptorSetLayout DescriptorSetLayout;
};

class PVulkanDescriptorSet
{
public:
    void CreateDescriptorSet(PVulkanDescriptorSetLayout* DescriptorSetLayout);
    void DestroyDescriptorSet();

    VkDescriptorSet GetVkDescriptorSet() const;
    
    std::span<SDescriptorSetBinding> GetBindings();
    std::span<const SDescriptorSetBinding> GetBindings() const;

private:
    std::vector<SDescriptorSetBinding> Bindings;

	VkDescriptorSet DescriptorSet;
};