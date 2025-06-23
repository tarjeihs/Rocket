#pragma once

#include "RHI/Public/Common/RHICommandList.h"

class FVulkanCommandListContext final : public IRHICommandListContext
{
public:
    explicit FVulkanCommandListContext(VkCommandBuffer InCommandBuffer)
        : CommandBuffer(InCommandBuffer)
    {
    }

    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) override;

    void RHIBeginRendering(VkImageView ImageView);
    void RHIEndRendering();
    void RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout);

private:
    VkCommandBuffer CommandBuffer;
};

struct FVulkanCommandSetMemoryBarrier : TRHICommand<FVulkanCommandSetMemoryBarrier>
{
    VkImage Image;
    VkImageLayout OldLayout;
    VkImageLayout NewLayout;

    FVulkanCommandSetMemoryBarrier(VkImage InImage, VkImageLayout InOldLayout, VkImageLayout InNewLayout)
        : Image(InImage), OldLayout(InOldLayout), NewLayout(InNewLayout)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHISetMemoryBarrier(Image, OldLayout, NewLayout);
    }
};

struct FVulkanCommandBeginRendering : TRHICommand<FVulkanCommandBeginRendering>
{
    VkImageView ImageView;

    FVulkanCommandBeginRendering(VkImageView InView)
        : ImageView(InView)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBeginRendering(ImageView);
    }
};

struct FVulkanCommandEndRendering : TRHICommand<FVulkanCommandEndRendering>
{
    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIEndRendering();
    }
};