#include "EnginePCH.h"
#include "VulkanTexture2D.h"

#include "stb_image.h"

#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSampler.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Types/UniquePtr.h"

void FVkTexture2D::Initialize(FTexture2DCreateInfo CreateInfo)
{
    //uint32 Width = 0, Height = 0, Channels = 0, Size = 0;

    EImageFormat ImageFormat = EImageFormat::None;
    if (CreateInfo.Bits == 8)
    {
        switch (CreateInfo.Components)
        {
            case 1: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R8_SRGB : EImageFormat::R8_UNORM);  break;
            case 2: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R8G8_SRGB : EImageFormat::R8G8_UNORM); break;
            case 3: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R8G8B8_SRGB : EImageFormat::R8G8B8_UNORM); break;
            case 4: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R8G8B8A8_SRGB : EImageFormat::R8G8B8A8_UNORM); break;
        }
    }
    else if (CreateInfo.Bits == 16)
    {
        switch (CreateInfo.Components)
        {
            case 1: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R16_SFLOAT : EImageFormat::R16_UNORM); break;
            case 2: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R16G16_SFLOAT : EImageFormat::R16G16_UNORM); break;
            case 3: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R16G16B16_SFLOAT : EImageFormat::R16G16B16_UNORM); break;
            case 4: ImageFormat = (CreateInfo.Format == ETexture2DFormat::SRGB ? EImageFormat::R16G16B16A16_SFLOAT : EImageFormat::R16G16B16A16_UNORM); break;
        }
    }

    //unsigned char* Data = stbi_load(CreateInfo.Path.GetData(), Cast<int32>(&Width), Cast<int32>(&Height), Cast<int32>(&Channels), Size);

    FImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.Layout = EImageLayout::ReadOnly;
    ImageCreateInfo.UsageFlags = EImageUsage::Sampled | EImageUsage::TransferDst;
    ImageCreateInfo.AspectFlags = EImageAspect::Color;
    ImageCreateInfo.Extent = { CreateInfo.Width, CreateInfo.Height };
    ImageCreateInfo.Format = ImageFormat;

    FVkSamplerCreateInfo SamplerCreateInfo;

    FVkImage* Image = new FVkImage();
    Image->Initialize(ImageCreateInfo);

    FVkSampler* Sampler = new FVkSampler();
    Sampler->Initialize(SamplerCreateInfo);

    FBufferCreateInfo BufferCreateInfo =
    {
        EBufferUsageFlag::None,
        EBufferTransferFlag::Read,
        EBufferMemoryFlag::Host,
        (SizeType)CreateInfo.Width * (SizeType)CreateInfo.Height * CreateInfo.Components
    };

    TUniquePtr<FVkBuffer> StagingBuffer = MakeUnique<FVkBuffer>();
    StagingBuffer->Initialize(BufferCreateInfo);

    void* MappedData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Info.Allocation, &MappedData);
    memcpy(MappedData, CreateInfo.Data, CreateInfo.Width * CreateInfo.Height * CreateInfo.Components);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Info.Allocation);
    
    GetRHI()->GetRenderer()->ImmediateSubmit([&](FVkCommandBuffer* CommandBuffer)
    {
        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        VkBufferImageCopy BufferImageCopy = {};
        BufferImageCopy.bufferOffset = 0;
        BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        BufferImageCopy.imageSubresource.mipLevel = 0;
        BufferImageCopy.imageSubresource.baseArrayLayer = 0;
        BufferImageCopy.imageSubresource.layerCount = 1;
        BufferImageCopy.imageExtent = { CreateInfo.Width, CreateInfo.Height, 1 };
        vkCmdCopyBufferToImage(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Info.Handle, Image->Info.ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
        
        Image->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    StagingBuffer->Shutdown();
    //stbi_image_free(Data);

    Info.Image = Image;
    Info.Sampler = Sampler;
}

void FVkTexture2D::Shutdown()
{
    Info.Image->Shutdown();
    Info.Sampler->Shutdown();

    delete Info.Image;
    delete Info.Sampler;
}

int32 FVkTexture2D::GetTextureID() const
{
    return 0;
}