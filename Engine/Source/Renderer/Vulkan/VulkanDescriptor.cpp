#include "VulkanDescriptor.h"

#include <vector>

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanDescriptorPool::CreatePool(PVulkanRHI* RHI, uint32_t MaxSets, std::span<SVulkanDescriptorPoolRatio> PoolRatios, uint32_t Flags)
{
	std::vector<VkDescriptorPoolSize> PoolSizes;
	for (SVulkanDescriptorPoolRatio PoolRatio : PoolRatios)
	{
		VkDescriptorPoolSize DescriptorPoolSize{};
		DescriptorPoolSize.type = PoolRatio.Type;
		DescriptorPoolSize.descriptorCount = PoolRatio.Ratio * MaxSets;
		PoolSizes.push_back(DescriptorPoolSize);
	}

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo{};
	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = Flags;
	DescriptorPoolCreateInfo.maxSets = MaxSets;
	DescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
	DescriptorPoolCreateInfo.pPoolSizes = PoolSizes.data();

	VkResult Result = vkCreateDescriptorPool(RHI->GetDevice()->GetVkDevice(), &DescriptorPoolCreateInfo, nullptr, &Pool);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor pool.");
}

void PVulkanDescriptorPool::DestroyPool(PVulkanRHI* RHI)
{
	vkDestroyDescriptorPool(RHI->GetDevice()->GetVkDevice(), Pool, nullptr);
}

void PVulkanDescriptorPool::ResetPool(PVulkanRHI* RHI)
{
	vkResetDescriptorPool(RHI->GetDevice()->GetVkDevice(), Pool, 0);
}

VkDescriptorPool PVulkanDescriptorPool::GetVkDescriptorPool() const
{
	return Pool;
}

void PVulkanDescriptorSet::CreateDescriptorSet(PVulkanRHI* RHI, PVulkanDescriptorSetLayout* DescriptorSetLayout)
{
	VkDescriptorSetLayout DescriptorSetLayoutPointer = DescriptorSetLayout->GetVkDescriptorSetLayout();

	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
	DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocInfo.pNext = nullptr;
	DescriptorSetAllocInfo.descriptorPool = RHI->GetMemory()->GetDescriptorPool()->GetVkDescriptorPool();
	DescriptorSetAllocInfo.descriptorSetCount = 1;
	DescriptorSetAllocInfo.pSetLayouts = &DescriptorSetLayoutPointer;

	VkResult Result = vkAllocateDescriptorSets(RHI->GetDevice()->GetVkDevice(), &DescriptorSetAllocInfo, &DescriptorSet);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor set.");
}

void PVulkanDescriptorSet::FreeDescriptorSet(PVulkanRHI* RHI)
{
	// Note: DescriptorPool must be created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT to allow for explicit, manual control.
	//VkResult Result = vkFreeDescriptorSets(RHI->GetDevice()->GetVkDevice(), RHI->GetMemory()->GetDescriptorPool()->GetVkDescriptorPool(), 1, &DescriptorSet);
	//RK_ASSERT(Result == VK_SUCCESS, "Failed to free descriptor set from memory.");
}

void PVulkanDescriptorSet::UseDescriptorStorageBuffer(PVulkanRHI* RHI, VkBuffer Buffer, VkDeviceSize Offset, VkDeviceSize Range, uint32_t Binding)
{
	VkDescriptorBufferInfo DescriptorBufferInfo{};
	DescriptorBufferInfo.buffer = Buffer;
	DescriptorBufferInfo.offset = 0;
	DescriptorBufferInfo.range = Range;

	VkWriteDescriptorSet WriteDescriptorSet{};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.dstBinding = Binding;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

	vkUpdateDescriptorSets(RHI->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
}

void PVulkanDescriptorSet::UseDescriptorUniformBuffer(PVulkanRHI* RHI, VkBuffer Buffer, VkDeviceSize Offset, VkDeviceSize Range, uint32_t Binding)
{
	VkDescriptorBufferInfo DescriptorBufferInfo{};
	DescriptorBufferInfo.buffer = Buffer;
	DescriptorBufferInfo.offset = 0;
	DescriptorBufferInfo.range = Range;

	VkWriteDescriptorSet WriteDescriptorSet{};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.dstBinding = Binding;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

	vkUpdateDescriptorSets(RHI->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
}

VkDescriptorSet PVulkanDescriptorSet::GetVkDescriptorSet() const
{
	return DescriptorSet;
}

void PVulkanDescriptorSetLayout::CreateDescriptorSetLayout(PVulkanRHI* RHI, std::vector<EDescriptorSetLayoutType> LayoutTypes)
{
	std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

	for (uint32_t Index = 0; Index < static_cast<uint32_t>(LayoutTypes.size()); ++Index) 
	{
		EDescriptorSetLayoutType LayoutType = LayoutTypes[Index];
		if (LayoutType == EDescriptorSetLayoutType::Storage)
		{
			DescriptorSetLayoutBindings.push_back(CreateStorageBufferBinding(Index));
		}
		else if (LayoutType == EDescriptorSetLayoutType::Uniform)
		{
			DescriptorSetLayoutBindings.push_back(CreateUniformBufferBinding(Index));
		}
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo{};
	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorSetLayoutBindings.size());
	DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings.data();

	VkResult Result = vkCreateDescriptorSetLayout(RHI->GetDevice()->GetVkDevice(), &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");
}

void PVulkanDescriptorSetLayout::FreeDescriptorSetLayout(PVulkanRHI* RHI)
{
	vkDestroyDescriptorSetLayout(RHI->GetDevice()->GetVkDevice(), DescriptorSetLayout, nullptr);
}

VkDescriptorSetLayout PVulkanDescriptorSetLayout::GetVkDescriptorSetLayout() const
{
	return DescriptorSetLayout;
}

VkDescriptorSetLayoutBinding PVulkanDescriptorSetLayout::CreateStorageBufferBinding(uint32_t Binding)
{
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding{};
	DescriptorSetLayoutBinding.binding = Binding;
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	DescriptorSetLayoutBinding.descriptorCount = 1;
	//DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	return DescriptorSetLayoutBinding;
}

VkDescriptorSetLayoutBinding PVulkanDescriptorSetLayout::CreateUniformBufferBinding(uint32_t Binding)
{
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding{};
	DescriptorSetLayoutBinding.binding = Binding;
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorSetLayoutBinding.descriptorCount = 1;
	//DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	return DescriptorSetLayoutBinding;
}
