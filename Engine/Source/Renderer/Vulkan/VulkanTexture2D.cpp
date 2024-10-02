#include "EnginePCH.h"
#include "VulkanTexture2D.h"

#include "stb_image.h"

#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSampler.h"

void PVulkanTexture2D::CreateTexture2D(unsigned char* Data)
{
    VkExtent2D Extent = { Width, Height };
    VkFormat ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    
    Image = new PVulkanImage();
    Image->Init(Extent, ImageFormat);  // Initialize the image with the extent and format
    Image->CreateImage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    PVulkanBuffer StagingBuffer = PVulkanBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    StagingBuffer.Allocate(Width * Height * Channels);

    void* MappedData = nullptr;
    vmaMapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), StagingBuffer.Allocation, &MappedData);
    memcpy(MappedData, Data, Width * Height * Channels);
    vmaUnmapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), StagingBuffer.Allocation);

    GetRHI()->GetSceneRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy BufferImageCopy = {};
        BufferImageCopy.bufferOffset = 0;
        BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        BufferImageCopy.imageSubresource.mipLevel = 0;
        BufferImageCopy.imageSubresource.baseArrayLayer = 0;
        BufferImageCopy.imageSubresource.layerCount = 1;
        BufferImageCopy.imageExtent = { Extent.width, Extent.height, 1 };

        vkCmdCopyBufferToImage(CommandBuffer->GetVkCommandBuffer(), StagingBuffer.Buffer, Image->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);

        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    StagingBuffer.Free();
}

void PVulkanTexture2D::DestroyTexture2D()
{
    Image->DestroyImage();
    Image->DestroyImageView();
    
    delete Image;
}

void PVulkanTexture2D::Deserialize(SBlob& Blob)
{
    unsigned char* Data = stbi_load_from_memory(Blob.Data.data(), static_cast<int>(Blob.Data.size()), &Width, &Height, &Channels, STBI_rgb_alpha);
    CreateTexture2D(Data);
    stbi_image_free(Data);
}

void PVulkanTexture2D::Cleanup()
{
    DestroyTexture2D();
}