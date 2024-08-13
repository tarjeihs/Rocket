#pragma once

#include <vector>
#include <span>
#include <vulkan/vulkan_core.h>

struct SDescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings;

	void AddBinding(uint32_t Binding, VkDescriptorType Type);
	void Clear();
	VkDescriptorSetLayout Build(VkDevice LogicalDevice, VkShaderStageFlags ShaderStages, void* Next = nullptr, VkDescriptorSetLayoutCreateFlags Flags = 0);
};

struct SDescriptorAllocator
{
	struct SPoolSizeRatio
	{
		VkDescriptorType Type;
		float Ratio;
	};

	VkDescriptorPool Pool;

	void InitializePool(VkDevice LogicalDevice, uint32_t MaxSets, std::span<SPoolSizeRatio> PoolRatios);
	void DeinitializePool(VkDevice LogicalDevice);
	void ClearDescriptors(VkDevice LogicalDevice);

	VkDescriptorSet Allocate(VkDevice LogicalDevice, VkDescriptorSetLayout Layout);
};