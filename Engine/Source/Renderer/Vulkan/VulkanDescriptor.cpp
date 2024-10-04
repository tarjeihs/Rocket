#include "EnginePCH.h"
#include "VulkanDescriptor.h"

#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanDescriptorPool::CreatePool(uint32_t MaxSets, std::span<SVulkanDescriptorPoolRatio> PoolRatios, uint32_t Flags)
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

	VkResult Result = vkCreateDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorPoolCreateInfo, nullptr, &Pool);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor pool.");
}

void PVulkanDescriptorPool::DestroyPool()
{
	vkDestroyDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), Pool, nullptr);
}

void PVulkanDescriptorPool::ResetPool()
{
	vkResetDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), Pool, 0);
}

VkDescriptorPool PVulkanDescriptorPool::GetVkDescriptorPool() const
{
	return Pool;
}

void PVulkanDescriptorSetLayout::CreateDescriptorSetLayout(const std::vector<SDescriptorSetBindingLayout>& Data)
{
	std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

	for (const auto& Binding : Data)
	{
		VkDescriptorType DescriptorType;
		switch (Binding.Type)
		{
			case EDescriptorSetBindingType::Uniform:
			{
				DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				break;
			}
			case EDescriptorSetBindingType::Storage:
			{
				DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			}
		}

		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding = {};
		DescriptorSetLayoutBinding.binding = Binding.Binding;
		DescriptorSetLayoutBinding.descriptorType = DescriptorType;
		DescriptorSetLayoutBinding.descriptorCount = 1;
		DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		DescriptorSetLayoutBindings.push_back(DescriptorSetLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo{};
	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorSetLayoutBindings.size());
	DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings.data();

	VkResult Result = vkCreateDescriptorSetLayout(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");	

	Bindings = Data;
}

void PVulkanDescriptorSetLayout::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(GetRHI()->GetDevice()->GetVkDevice(), DescriptorSetLayout, nullptr);
}

VkDescriptorSetLayout PVulkanDescriptorSetLayout::GetVkDescriptorSetLayout() const
{
	return DescriptorSetLayout;
}

std::span<SDescriptorSetBindingLayout> PVulkanDescriptorSetLayout::GetBindings()
{
    return std::span<SDescriptorSetBindingLayout>(Bindings.data(), Bindings.size());
}

std::span<const SDescriptorSetBindingLayout> PVulkanDescriptorSetLayout::GetBindings() const
{
    return std::span<const SDescriptorSetBindingLayout>(Bindings.data(), Bindings.size());
}

void PVulkanDescriptorSet::CreateDescriptorSet(PVulkanDescriptorSetLayout* DescriptorSetLayout, PVulkanFrame* Frame)
{
	VkDescriptorSetLayout DescriptorSetLayoutPointer = DescriptorSetLayout->GetVkDescriptorSetLayout();

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {};
	DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInfo.pNext = nullptr;
	DescriptorSetAllocateInfo.descriptorPool = Frame->GetMemory()->GetDescriptorPool()->GetVkDescriptorPool();
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &DescriptorSetLayoutPointer;

	VkResult Result = vkAllocateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorSetAllocateInfo, &DescriptorSet);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor set.");

	for (SDescriptorSetBindingLayout& BindingLayout : DescriptorSetLayout->GetBindings())
	{
		switch (BindingLayout.Type)
		{
			case EDescriptorSetBindingType::Storage: 
			{
				PVulkanBuffer* Buffer = new PVulkanBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
				Buffer->Allocate(1024 * 10 * 10);
				
				SDescriptorSetBinding Binding;
				Binding.Layout = &BindingLayout;
				Binding.Data = Buffer;
				Bindings.push_back(Binding);
				
				VkDescriptorBufferInfo DescriptorBufferInfo{};
				DescriptorBufferInfo.buffer = Buffer->Buffer;
				DescriptorBufferInfo.offset = 0;
				DescriptorBufferInfo.range = VK_WHOLE_SIZE;

				VkWriteDescriptorSet WriteDescriptorSet{};
				WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				WriteDescriptorSet.dstSet = DescriptorSet;
				WriteDescriptorSet.dstBinding = BindingLayout.Binding;
				WriteDescriptorSet.dstArrayElement = 0;
				WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				WriteDescriptorSet.descriptorCount = 1;
				WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

				vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
				break;
			}
			case EDescriptorSetBindingType::Uniform:
			{
				PVulkanBuffer* Buffer = new PVulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
				Buffer->Allocate(BindingLayout.Size);
				
				SDescriptorSetBinding Binding;
				Binding.Layout = &BindingLayout;
				Binding.Data = Buffer;
				Bindings.push_back(Binding);
				
				VkDescriptorBufferInfo DescriptorBufferInfo{};
				DescriptorBufferInfo.buffer = Buffer->Buffer;
				DescriptorBufferInfo.offset = 0;
				DescriptorBufferInfo.range = BindingLayout.Size;

				VkWriteDescriptorSet WriteDescriptorSet{};
				WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				WriteDescriptorSet.dstSet = DescriptorSet;
				WriteDescriptorSet.dstBinding = BindingLayout.Binding;
				WriteDescriptorSet.dstArrayElement = 0;
				WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				WriteDescriptorSet.descriptorCount = 1;
				WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

				vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
				break;
			}
		}
	}
}

void PVulkanDescriptorSet::DestroyDescriptorSet()
{
	for (auto& Binding : Bindings)
	{
		switch (Binding.Layout->Type)
		{
			case EDescriptorSetBindingType::Uniform:
			case EDescriptorSetBindingType::Storage:
			{
				PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
				Buffer->Free();
				break;
			}
		}
	}
}

VkDescriptorSet PVulkanDescriptorSet::GetVkDescriptorSet() const
{
	return DescriptorSet;
}

std::span<SDescriptorSetBinding> PVulkanDescriptorSet::GetBindings()
{
    return std::span<SDescriptorSetBinding>(Bindings.data(), Bindings.size());
}

std::span<const SDescriptorSetBinding> PVulkanDescriptorSet::GetBindings() const 
{
    return std::span<const SDescriptorSetBinding>(Bindings.data(), Bindings.size());
}