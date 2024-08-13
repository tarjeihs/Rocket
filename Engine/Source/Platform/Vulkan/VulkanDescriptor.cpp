#include "EnginePCH.h"
#include "VulkanDescriptor.h"

#include "Core/Assert.h"

void SDescriptorLayoutBuilder::AddBinding(uint32_t Binding, VkDescriptorType Type)
{
	VkDescriptorSetLayoutBinding LayoutBinding{};
	LayoutBinding.binding = Binding;
	LayoutBinding.descriptorCount = 1;
	LayoutBinding.descriptorType = Type;

	Bindings.push_back(LayoutBinding);
}

void SDescriptorLayoutBuilder::Clear()
{
	Bindings.clear();
}

VkDescriptorSetLayout SDescriptorLayoutBuilder::Build(VkDevice LogicalDevice, VkShaderStageFlags ShaderStages, void* Next, VkDescriptorSetLayoutCreateFlags Flags)
{
	for (auto& b : Bindings)
	{
		b.stageFlags |= ShaderStages;
	}

	VkDescriptorSetLayoutCreateInfo LayoutCreateInfo{};
	LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutCreateInfo.pNext = Next;
	LayoutCreateInfo.pBindings = Bindings.data();
	LayoutCreateInfo.bindingCount = (uint32_t)Bindings.size();
	LayoutCreateInfo.flags = Flags;

	VkDescriptorSetLayout DescriptorSetLayout;
	VkResult Result = vkCreateDescriptorSetLayout(LogicalDevice, &LayoutCreateInfo, nullptr, &DescriptorSetLayout);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");

	return DescriptorSetLayout;
}

void SDescriptorAllocator::InitializePool(VkDevice LogicalDevice, uint32_t MaxSets, std::span<SPoolSizeRatio> PoolRatios)
{
	std::vector<VkDescriptorPoolSize> PoolSizes;
	for (SPoolSizeRatio PoolSizeRatio : PoolRatios)
	{
		VkDescriptorPoolSize PoolSize{};
		PoolSize.type = PoolSizeRatio.Type;
		PoolSize.descriptorCount = uint32_t(PoolSizeRatio.Ratio * MaxSets);

		PoolSizes.push_back(PoolSize);
	}

	VkDescriptorPoolCreateInfo PoolCreateInfo{};
	PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolCreateInfo.flags = 0;
	PoolCreateInfo.maxSets = MaxSets;
	PoolCreateInfo.poolSizeCount = (uint32_t)PoolSizes.size();
	PoolCreateInfo.pPoolSizes = PoolSizes.data();

	VkResult Result = vkCreateDescriptorPool(LogicalDevice, &PoolCreateInfo, nullptr, &Pool);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor pool.");
}

void SDescriptorAllocator::DeinitializePool(VkDevice LogicalDevice)
{
	vkDestroyDescriptorPool(LogicalDevice, Pool, nullptr);
}

void SDescriptorAllocator::ClearDescriptors(VkDevice LogicalDevice)
{
	vkResetDescriptorPool(LogicalDevice, Pool, 0);
}

VkDescriptorSet SDescriptorAllocator::Allocate(VkDevice LogicalDevice, VkDescriptorSetLayout Layout)
{
	VkDescriptorSetAllocateInfo AllocInfo{};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.pNext = nullptr;
	AllocInfo.descriptorPool = Pool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &Layout;

	VkDescriptorSet DescriptorSet;
	VkResult Result = vkAllocateDescriptorSets(LogicalDevice, &AllocInfo, &DescriptorSet);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor sets.");

	return DescriptorSet;
}