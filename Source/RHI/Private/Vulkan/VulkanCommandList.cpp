#include "RocketPCH.h"
#include "RHI/Private/Vulkan/VulkanCommandList.h"

#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"
#include "VulkanCommandList.h"

void FVulkanCommandListContext::RHISetScissorRect(uint32_t X, uint32_t Y)
{
}

void FVulkanCommandListContext::RHIBeginRendering(VkImageView ImageView)
{
    VkRenderingAttachmentInfoKHR ColorAttachmentInfo = {};
    ColorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    ColorAttachmentInfo.imageView        = ImageView;
    ColorAttachmentInfo.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Fixed layout
    ColorAttachmentInfo.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachmentInfo.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachmentInfo.clearValue.color = {{ 1, 1, 0, 1 }};

    VkRenderingInfo RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments    = &ColorAttachmentInfo;
    RenderingInfo.layerCount           = 1;
    RenderingInfo.viewMask             = 0;
    RenderingInfo.renderArea.extent.width  = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
    RenderingInfo.renderArea.extent.height = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height;

    vkCmdBeginRendering(CommandBuffer, &RenderingInfo);
}

void FVulkanCommandListContext::RHIEndRendering()
{
    vkCmdEndRendering(CommandBuffer);
}

void FVulkanCommandListContext::RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.oldLayout = OldLayout;
    ImageMemoryBarrier.newLayout = NewLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = Image;
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;
    ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    
    vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
}
