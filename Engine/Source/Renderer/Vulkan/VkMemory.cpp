#include "EnginePCH.h"
#include "VkMemory.h"

#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

void FVkMemory::Initialize()
{
	Info.DescriptorPool = new FVkDescriptorPool();
	Info.DescriptorSetLayout = new FVkDescriptorSetLayout();
	Info.PipelineLayout = new FVkPipelineLayout();

	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo =
	{
		{
			{ EVkDescriptorType::StructuredBuffer, 	16 },
			{ EVkDescriptorType::RWTexture2D, 		16 },
			{ EVkDescriptorType::Texture2D, 		16 },
		}
	};

	FVkDescriptorSetCreateInfo DescriptorSetCreateInfo =
	{
		Info.DescriptorPool,
		Info.DescriptorSetLayout
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo =
	{
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			16.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 			16.0f },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 	16.0f },
		},
		1,
		VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
	};

	FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo =
	{
		{
			Info.DescriptorSetLayout
		}
	};

	Info.DescriptorPool->Initialize(DescriptorPoolCreateInfo);
	Info.DescriptorSetLayout->Initialize(DescriptorSetLayoutCreateInfo);
	Info.PipelineLayout->Initialize(PipelineLayoutCreateInfo);

	FRendererContext CreateInfo;
	GRenderer->CreateRendererContext(CreateInfo);

	for (SizeType FrameIndex = 0; FrameIndex < CONCURRENT_FRAME_COUNT; ++FrameIndex)
	{
		FVkMemoryInfo::FFrame& Frame = Info.Frames[FrameIndex];
		
		Frame.DescriptorSet = new FVkDescriptorSet();
		Frame.DescriptorSet->Initialize(DescriptorSetCreateInfo);

		for (SizeType Index = 0; Index < CreateInfo.BufferCreateInfos.GetSize(); ++Index)
		{
			FVkBuffer* Buffer = new FVkBuffer();
			Buffer->Initialize(CreateInfo.BufferCreateInfos[Index].Value);
			Frame.Buffers.Insert(CreateInfo.BufferCreateInfos[Index].Key, Buffer);
			Frame.DescriptorSet->WriteBuffer(0, Index, Buffer);
		}

		for (SizeType Index = 0; Index < CreateInfo.ImageCreateInfos.GetSize(); ++Index)
		{
			FVkImage* Image = new FVkImage();
			Image->Initialize(CreateInfo.ImageCreateInfos[Index].Value);
			Frame.Images.Insert(CreateInfo.ImageCreateInfos[Index].Key, Image);
			Frame.DescriptorSet->WriteImage(1, Index, Image);
		}
	}
}

void FVkMemory::Shutdown()
{
	
}

void FVkMemory::Execute()
{
	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Info.PipelineLayout->Info.Handle, 0, 1, &Info.Frames[GetRHI()->GetRenderer()->GetFrameIndex()].DescriptorSet->Info.Handle, 0, 0);
	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Info.PipelineLayout->Info.Handle, 0, 1, &Info.Frames[GetRHI()->GetRenderer()->GetFrameIndex()].DescriptorSet->Info.Handle, 0, 0);
}

void FVkMemory::AddBuffer(const FString& Name, const FBufferCreateInfo& CreateInfo, uint32 Frame)
{
}

void FVkMemory::RemoveBuffer(const FString& Name, uint32 Frame)
{
}

IBuffer* FVkMemory::GetBuffer(const FString& Name, uint32 Frame) const
{
	return nullptr;
}

void FVkMemory::BindBuffer(const FString& Name, uint32 Binding, uint32 Frame)
{
}

void FVkMemory::AddImage(const FString& Name, const FImageCreateInfo& CreateInfo, uint32 Frame)
{
}

void FVkMemory::RemoveImage(const FString& Name, uint32 Frame)
{
}

IImage* FVkMemory::GetImage(const FString& Name, uint32 Frame) const
{
	return nullptr;
}

void FVkMemory::BindImage(const FString& Name, uint32 Binding, uint32 Frame)
{
}
