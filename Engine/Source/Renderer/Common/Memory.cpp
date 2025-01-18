#include "EnginePCH.h"
#include "Memory.h"

#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"

void FVkMemory::Initialize()
{
	Info.DescriptorPool = new FVkDescriptorPool();
	Info.DescriptorSet = new FVkDescriptorSet();
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
	Info.DescriptorSet->Initialize(DescriptorSetCreateInfo);
	Info.PipelineLayout->Initialize(PipelineLayoutCreateInfo);

	FVkImageCreateInfo IntermediateColorAttachmentCreateInfo =
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_R16G16B16A16_SFLOAT
	};

	FVkImageCreateInfo PresentColorAttachmentCreateInfo =
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_B8G8R8A8_SRGB
	};

	FVkImageCreateInfo DepthAttachmentCreateInfo =
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_D32_SFLOAT
	};
}

void FVkMemory::Shutdown()
{
}

void FVkMemory::AddBuffer(FBufferCreateInfo& CreateInfo, const FString& Name, uint32 Frame)
{
	RK_ASSERT(Info.Buffer.Find(Name) == nullptr, "A buffer with this name already exists.");

	TDoubleLinkedList<IBuffer*> LinkedList;

	for (SizeType Index = 0; Index < Frame; ++Index)
	{
		FVkBuffer* Buffer = new FVkBuffer();
		Buffer->Initialize(CreateInfo);
		LinkedList.PushBack(Buffer);
	}

	Info.Buffer.Insert(Name, MoveTemp(LinkedList));
	// Issue is that the underlying type (TOptional) of TMap internal array, does not support moving.
	//RK_ASSERT(LinkedList.IsEmpty(), "Is empty...");
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

void FVkMemory::AddImage(FImageCreateInfo& CreateInfo, const FString& Name, uint32 Frame)
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
