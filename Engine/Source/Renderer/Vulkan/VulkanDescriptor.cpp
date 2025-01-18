#include "Core/Logger.h"
#include "EnginePCH.h"
#include "VulkanDescriptor.h"

#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanTexture2D.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSampler.h"
#include "Utils/Profiler.h"

namespace Utils
{
	VkDescriptorType GetVkDescriptorType(EVkDescriptorType DescriptorType)
	{
		switch (DescriptorType)
		{
			case EVkDescriptorType::StructuredBuffer: 	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case EVkDescriptorType::RWTexture2D: 		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case EVkDescriptorType::Sampler: 			return VK_DESCRIPTOR_TYPE_SAMPLER;
			case EVkDescriptorType::Texture2D: 			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

    VkBufferUsageFlags GetBufferUsageFlags(EVkDescriptorType DescriptorType)
    {
		switch (DescriptorType)
		{
			case EVkDescriptorType::StructuredBuffer: 	return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::RWTexture2D: 		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::Sampler: 			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case EVkDescriptorType::Texture2D: 			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		return UINT32_MAX;
    }

	VmaMemoryUsage GetMemoryUsageFlags(EVkDescriptorType DescriptorType)
	{
		switch (DescriptorType)
		{
			case EVkDescriptorType::StructuredBuffer: 	return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::RWTexture2D: 		return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::Sampler: 			return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case EVkDescriptorType::Texture2D: 			return VMA_MEMORY_USAGE_CPU_TO_GPU;
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

	VkResult Result = vkCreateDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorPoolCreateInfo, nullptr, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor pool.");
}

void FVkDescriptorPool::Shutdown()
{
	vkDestroyDescriptorPool(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, nullptr);
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

void FVkDescriptorSetLayout::Shutdown()
{
	vkDestroyDescriptorSetLayout(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, nullptr);
}

void FVkDescriptorSet::Initialize(FVkDescriptorSetCreateInfo& CreateInfo)
{
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {};
	DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInfo.pNext = nullptr;
	DescriptorSetAllocateInfo.descriptorPool = CreateInfo.DescriptorPool->Info.Handle;
	DescriptorSetAllocateInfo.descriptorSetCount = 1;
	DescriptorSetAllocateInfo.pSetLayouts = &CreateInfo.DescriptorSetLayout->Info.Handle;

	VkResult Result = vkAllocateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), &DescriptorSetAllocateInfo, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor set.");
}

void FVkDescriptorSet::Shutdown()
{
	for (SizeType Frame = 0; Frame < CONCURRENT_FRAME_COUNT; ++Frame)
	{
		for (SizeType Index = 0; Index < Info.Buffers.GetSize(); ++Index)
		{
			Info.Buffers[Index][Frame]->Shutdown();
		}

		for (SizeType Index = 0; Index < Info.Images.GetSize(); ++Index)
		{
			Info.Images[Index][Frame]->Shutdown();
		}
	}

	for (SizeType Index = 0; Index < Info.Textures.GetSize(); ++Index)
	{
		Info.Textures[Index]->Shutdown();
	}
}

void FVkDescriptorSet::WriteBuffer(uint32 Binding, uint32 Index, FVkBuffer* Buffer)
{
	PROFILE_FUNC_SCOPE("FVkDescriptorSet::WriteBuffer")

	VkDescriptorBufferInfo DescriptorBufferInfo = {};
	DescriptorBufferInfo.buffer = Buffer->Info.Handle;
	DescriptorBufferInfo.range = VK_WHOLE_SIZE;
	DescriptorBufferInfo.offset = 0;

	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = Info.Handle;
	WriteDescriptorSet.dstBinding = Binding;
	WriteDescriptorSet.dstArrayElement = Index;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

	vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
}

void FVkDescriptorSet::WriteTexture2D(uint32 Binding, uint32 Index, FVkTexture2D* Texture2D)
{
	PROFILE_FUNC_SCOPE("FVkDescriptorSet::WriteTexture2D")

    VkDescriptorImageInfo DescriptorImageInfo = {};
    DescriptorImageInfo.imageView = Texture2D->Info.Image->Info.ImageViewHandle;
    DescriptorImageInfo.sampler = Texture2D->Info.Sampler->Info.Handle;
    DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet WriteDescriptorSet = {};
    WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSet.dstSet = Info.Handle;
    WriteDescriptorSet.dstBinding = Binding;
    WriteDescriptorSet.dstArrayElement = Index;
    WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteDescriptorSet.descriptorCount = 1;
    WriteDescriptorSet.pImageInfo = &DescriptorImageInfo;

    vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
}

void FVkDescriptorSet::WriteImage(uint32 Binding, uint32 Index, FVkImage* Image)
{
	PROFILE_FUNC_SCOPE("FVkDescriptorSet::WriteImage")
	
	VkDescriptorImageInfo DescriptorImageInfo = {};
	DescriptorImageInfo.imageView = Image->Info.ImageViewHandle;
	DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	DescriptorImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = Info.Handle;
	WriteDescriptorSet.dstBinding = Binding;
	WriteDescriptorSet.dstArrayElement = Index;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.pImageInfo = &DescriptorImageInfo;

	vkUpdateDescriptorSets(GetRHI()->GetDevice()->GetVkDevice(), 1, &WriteDescriptorSet, 0, nullptr);
}