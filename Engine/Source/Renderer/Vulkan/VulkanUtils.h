#pragma once

static void SetImageLayout(VkImage Image, VkCommandBuffer CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout)
{
    VkImageSubresourceRange SubresourceRange = {};
    SubresourceRange.aspectMask = (NewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    SubresourceRange.baseMipLevel = 0;
    SubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    SubresourceRange.baseArrayLayer = 0;
    SubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageMemoryBarrier2 ImageMemoryBarrier = {};
    ImageMemoryBarrier.pNext = nullptr;
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    ImageMemoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    ImageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    ImageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    ImageMemoryBarrier.oldLayout = CurrentLayout;
    ImageMemoryBarrier.newLayout = NewLayout;
    ImageMemoryBarrier.subresourceRange = SubresourceRange;
    ImageMemoryBarrier.image = Image;

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.pNext = nullptr;
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &ImageMemoryBarrier;

    vkCmdPipelineBarrier2(CommandBuffer, &DependencyInfo);
}
