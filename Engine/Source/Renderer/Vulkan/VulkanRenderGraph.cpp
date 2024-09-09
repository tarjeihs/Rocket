#include "EnginePCH.h"
#include "VulkanRenderGraph.h"

#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanCommand.h"

void PVulkanRenderGraph::AddCommand(std::function<void(PVulkanFrame*)>&& Func)
{
    Commands.push_back(std::move(Func));
}

void PVulkanRenderGraph::BeginRendering() 
{
    VkRenderingAttachmentInfo ColorAttachment{};
    ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    ColorAttachment.pNext = nullptr;
    ColorAttachment.imageView = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkImageView();
    ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingAttachmentInfo DepthAttachment{};
    DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    DepthAttachment.pNext = nullptr;
    DepthAttachment.imageView = GetRHI()->GetSceneRenderer()->GetDepthImage()->GetVkImageView();
    DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthAttachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo RenderingInfo{};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    RenderingInfo.pNext = nullptr;
    RenderingInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, VkExtent2D { GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width, GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height }};
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;
    RenderingInfo.pDepthAttachment = &DepthAttachment;
    RenderingInfo.pStencilAttachment = nullptr;

    VkViewport Viewport{};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width;
    Viewport.height = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor = {};
    Scissor.offset.x = 0;
    Scissor.offset.y = 0;
    Scissor.extent.width = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width;
    Scissor.extent.height = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height;

    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetParallelFramePool()->GetCurrentFrame();
    vkCmdBeginRendering(Frame->CommandBuffer->GetVkCommandBuffer(), &RenderingInfo);
    vkCmdSetViewport(Frame->CommandBuffer->GetVkCommandBuffer(), 0, 1, &Viewport);
    vkCmdSetScissor(Frame->CommandBuffer->GetVkCommandBuffer(), 0, 1, &Scissor);
}

void PVulkanRenderGraph::EndRendering()
{
    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetParallelFramePool()->GetCurrentFrame();
    vkCmdEndRendering(Frame->CommandBuffer->GetVkCommandBuffer());   
}

void PVulkanRenderGraph::Execute(PVulkanFrame* Frame)
{
    for (const auto& Command : Commands)
    {
        Command(Frame);
    }
}