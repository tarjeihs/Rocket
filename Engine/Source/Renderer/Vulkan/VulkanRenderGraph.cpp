#include "EnginePCH.h"
#include "VulkanRenderGraph.h"

#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanCommand.h"

void PVulkanRenderGraph::AddCommand(std::function<void(PVulkanCommandBuffer*)>&& Func)
{
    Commands.push_back(std::move(Func));
}

void PVulkanRenderGraph::BeginRendering() 
{
    PROFILE_FUNC_SCOPE("PVulkanRenderGraph::BeginRendering")

    VkRenderingAttachmentInfo ColorAttachment{};
    ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    ColorAttachment.pNext = nullptr;
    ColorAttachment.imageView = GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetVkImageView();
    ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.clearValue = { 0.0033f, 0.0033f, 0.0033f, 1.0f };

    VkRenderingAttachmentInfo DepthAttachment{};
    DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    DepthAttachment.pNext = nullptr;
    DepthAttachment.imageView = GetRHI()->GetRenderer()->GetDepthAttachmentImage()->GetVkImageView();
    DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthAttachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo RenderingInfo{};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    RenderingInfo.pNext = nullptr;
    RenderingInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, VkExtent2D { GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().width, GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().height }};
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;
    RenderingInfo.pDepthAttachment = &DepthAttachment;
    RenderingInfo.pStencilAttachment = nullptr;

    VkViewport Viewport{};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().width;
    Viewport.height = GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor = {};
    Scissor.offset.x = 0;
    Scissor.offset.y = 0;
    Scissor.extent.width = GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().width;
    Scissor.extent.height = GetRHI()->GetRenderer()->GetColorAttachmentImage()->GetImageExtent2D().height;

    vkCmdBeginRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
    vkCmdSetViewport(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Viewport);
    vkCmdSetScissor(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Scissor);
}

void PVulkanRenderGraph::EndRendering()
{
    PROFILE_FUNC_SCOPE("PVulkanRenderGraph::EndRendering")
    
    vkCmdEndRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer());   
}

void PVulkanRenderGraph::Execute(PVulkanCommandBuffer* CommandBuffer)
{
    PROFILE_FUNC_SCOPE("PVulkanRenderGraph::Execute")

    for (const auto& Command : Commands)
    {
        Command(CommandBuffer);
    }
}