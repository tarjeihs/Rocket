#include "EnginePCH.h"
#include "VulkanTexture2D.h"

#include "stb_image.h"

#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSampler.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Types/UniquePtr.h"

void FVkTexture2D::Initialize(FVkTexture2DCreateInfo& CreateInfo)
{
    unsigned char* Data = stbi_load(CreateInfo.Path.GetData(), Cast<int32>(&Info.Width), Cast<int32>(&Info.Height), Cast<int32>(&Info.Channels), STBI_rgb_alpha);

    FVkImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.ImageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    ImageCreateInfo.ImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ImageCreateInfo.ImageViewAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageCreateInfo.Extent = VkExtent2D(Info.Width, Info.Height);
    ImageCreateInfo.Format = CreateInfo.ImageFormat;

    FVkSamplerCreateInfo SamplerCreateInfo;

    FVkImage* Image = new FVkImage();
    Image->Initialize(ImageCreateInfo);

    FVkSampler* Sampler = new FVkSampler();
    Sampler->Initialize(SamplerCreateInfo);

    FVkBufferCreateInfo BufferCreateInfo =
    {
        .UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .MemoryUsageFlags = VMA_MEMORY_USAGE_CPU_ONLY,
        .Size = (SizeType)Info.Width * (SizeType)Info.Height * STBI_rgb_alpha
    };

    TUniquePtr<FVkBuffer> StagingBuffer = MakeUnique<FVkBuffer>();
    StagingBuffer->Initialize(BufferCreateInfo);

    void* MappedData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Info.Allocation, &MappedData);
    memcpy(MappedData, Data, Info.Width * Info.Height * Info.Channels);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Info.Allocation);
    
    GetRHI()->GetRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        VkBufferImageCopy BufferImageCopy = {};
        BufferImageCopy.bufferOffset = 0;
        BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        BufferImageCopy.imageSubresource.mipLevel = 0;
        BufferImageCopy.imageSubresource.baseArrayLayer = 0;
        BufferImageCopy.imageSubresource.layerCount = 1;
        BufferImageCopy.imageExtent = { Info.Width, Info.Height, 1 };
        vkCmdCopyBufferToImage(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Info.Handle, Image->Info.ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
        
        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    StagingBuffer->Free();
    stbi_image_free(Data);

    Info.Image = Image;
    Info.Sampler = Sampler;
}

void FVkTexture2D::Shutdown()
{
    Info.Image->Shutdown();
    Info.Sampler->Shutdown();
}