#include "EnginePCH.h"
#include "VulkanDescriptor.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Types/SharedPtr.h"

namespace Utils
{
	VkDescriptorType GetVkDescriptorType(EVkDescriptorType DescriptorType)
	{
		switch (DescriptorType)
		{
			case EVkDescriptorType::SSBO: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case EVkDescriptorType::SSIO: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case EVkDescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
			case EVkDescriptorType::SamplerImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		}
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

    VkBufferUsageFlags GetBufferUsageFlags(EVkDescriptorType DescriptorType)
    {
		switch (DescriptorType)
		{
			case EVkDescriptorType::SSBO: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::SSIO: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::Sampler: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::SamplerImage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		return UINT32_MAX;
    }

	VmaMemoryUsage GetMemoryUsageFlags(EVkDescriptorType DescriptorType)
	{
		switch (DescriptorType)
		{
			case EVkDescriptorType::SSBO: return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::SSIO: return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::Sampler: return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::SamplerImage: return VMA_MEMORY_USAGE_CPU_TO_GPU;
		}
		return VMA_MEMORY_USAGE_UNKNOWN;
	}
}

void FVkDescriptorPool::Initialize(FVkDescriptorPoolCreateInfo& CreateInfo)
{
	TArray<VkDescriptorPoolSize> PoolSizes;
	
	for (FVkDescriptorPoolRatio& PoolRatio : CreateInfo.PoolRatios)
	{
		VkDescriptorPoolSize DescriptorPoolSize{};
		DescriptorPoolSize.type = PoolRatio.Type;
		DescriptorPoolSize.descriptorCount = PoolRatio.Ratio * CreateInfo.MaxSetCount;
		
		PoolSizes.Add(DescriptorPoolSize);
	}

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo{};
	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.flags = CreateInfo.Flags;
	DescriptorPoolCreateInfo.maxSets = CreateInfo.MaxSetCount;
	DescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.GetSize());
	DescriptorPoolCreateInfo.pPoolSizes = PoolSizes.GetData();

	VkResult Result = vkCreateDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorPoolCreateInfo, nullptr, &Info.DescriptorPool);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor pool.");
}

void FVkDescriptorPool::Destroy()
{
	vkDestroyDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), Info.DescriptorPool, nullptr);
}

void FVkDescriptorSetLayout::Initialize(FVkDescriptorSetLayoutCreateInfo& CreateInfo)
{
	TArray<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;
	TArray<VkDescriptorBindingFlags> DescriptorBindingFlags;

	for (uint32_t Index = 0; Index < CreateInfo.Descriptors.GetSize(); ++Index)
	{
		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding = {};
		DescriptorSetLayoutBinding.binding = Index;
		DescriptorSetLayoutBinding.descriptorType = Utils::GetVkDescriptorType(CreateInfo.Descriptors[Index].DescriptorType);
		DescriptorSetLayoutBinding.descriptorCount = CreateInfo.Descriptors[Index].DescriptorCount;
		DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

		DescriptorSetLayoutBindings.Add(DescriptorSetLayoutBinding);
		DescriptorBindingFlags.Add(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
	}

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT DescriptorSetLayoutBindingExtraCreateInfo = {};
	DescriptorSetLayoutBindingExtraCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	DescriptorSetLayoutBindingExtraCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorBindingFlags.GetSize());
	DescriptorSetLayoutBindingExtraCreateInfo.pBindingFlags = DescriptorBindingFlags.GetData();

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo{};
	DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorSetLayoutBindings.GetSize());
	DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings.GetData();
	DescriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
	DescriptorSetLayoutCreateInfo.pNext = &DescriptorSetLayoutBindingExtraCreateInfo;

	VkResult Result = vkCreateDescriptorSetLayout(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorSetLayoutCreateInfo, nullptr, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");	
}

void FVkDescriptorSetLayout::Destroy()
{
	vkDestroyDescriptorSetLayout(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, nullptr);
}

void FVkDescriptorSet::Initialize(FVkDescriptorSetCreateInfo& CreateInfo)
{
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {};
	DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInfo.pNext = nullptr;
	DescriptorSetAllocateInfo.descriptorPool = CreateInfo.DescriptorPool->Info.DescriptorPool;
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &CreateInfo.DescriptorSetLayout->Info.Handle;

	VkResult Result = vkAllocateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorSetAllocateInfo, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor set.");
}

void FVkDescriptorSet::Destroy()
{
	for (SizeType Index = 0; Index < Info.Buffer.GetSize(); ++Index)
	{
		Info.Buffer[Index]->Free();
	}
	Info.DescriptorPool->Destroy();
}

void FVkDescriptorSet::WriteBuffer(uint32 Index, FVkBuffer* Buffer)
{
	VkDescriptorBufferInfo DescriptorBufferInfo = {};
	DescriptorBufferInfo.buffer = Buffer->Info.Handle;
	DescriptorBufferInfo.range = VK_WHOLE_SIZE;
	DescriptorBufferInfo.offset = 0;

	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = Info.Handle;
	WriteDescriptorSet.dstBinding = Index;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

	vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);

	Info.Buffer.Add(Buffer);
}

void FVkDescriptorSet::ReadBuffer(uint32 Index, FVkBuffer*& Buffer)
{
	FVkBuffer* Result = Info.Buffer[Index];
	if (Result)
	{
		Buffer = MoveTemp(Result);
	}
}

void FVkDescriptorSet::Bind(FVkPipelineLayout* PipelineLayout)
{
	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout->Info.Handle, 0, 1, &Info.Handle, 0, 0);
}

//void FVkDescriptorSet::WriteBuffer(uint32 Index, FVkBufferCreateInfo& CreateInfo)
//{
//	TSharedPtr<FVkBuffer> Buffer = MakeShared<FVkBuffer>();
//	Buffer->Initialize(CreateInfo);
//}