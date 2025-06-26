#include "RocketPCH.h"
#include "RHI/Private/Vulkan/VulkanRenderGraph.h"

#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"
#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"

void CreatePoolAllocator(FVulkanPoolAllocator& OutPoolAllocator, EPoolAllocatorType Type, VkDeviceSize Size)
{
    VkResult Result = VK_SUCCESS;

    VkPhysicalDeviceMemoryProperties MemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), &MemoryProperties);

    // All memory types are acceptable but regarded unsafe for production. Use vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements instead.
    // TODO: Query correct memory type for proper performance. Different hardware might differ in TypeIndex order.
    uint32_t MemoryTypeBits = 0xFFFFFFFF;
    uint32_t TypeIndex = UINT32_MAX;

    for (uint32_t Index = 0; Index < MemoryProperties.memoryTypeCount; ++Index)
    {
        if ((MemoryTypeBits & (1 << Index)) && (MemoryProperties.memoryTypes[Index].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            TypeIndex = Index;
            break;
        }
    }

    RK_ASSERT(TypeIndex != UINT32_MAX);

    VkMemoryAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = Size;
    AllocInfo.memoryTypeIndex = TypeIndex;

    Result = vkAllocateMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), &AllocInfo, nullptr, &OutPoolAllocator.PhysicalMemory);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate memory.");

    VmaVirtualBlockCreateInfo BlockCreateInfo = {};
    BlockCreateInfo.size = VK_WHOLE_SIZE;
    BlockCreateInfo.flags = 0;

    Result = vmaCreateVirtualBlock(&BlockCreateInfo, &OutPoolAllocator.VirtualMemory);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create virtual block.");
}

void DestroyPoolAllocator(FVulkanPoolAllocator& PoolAllocator)
{
    vkFreeMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), PoolAllocator.PhysicalMemory, nullptr);
    vmaClearVirtualBlock(PoolAllocator.VirtualMemory);
    vmaDestroyVirtualBlock(PoolAllocator.VirtualMemory);
}

void ResetPoolAllocator(FVulkanPoolAllocator &PoolAllocator)
{
    vmaClearVirtualBlock(PoolAllocator.VirtualMemory);
    PoolAllocator.Offset = 0;
}

void FVulkanRGBuilder::AddPass(std::string Name, EVulkanQueueType Queue, std::span<const FRGResourceAccess> Reads, std::span<const FRGResourceAccess> Writes, std::function<void(FRHICommandList&)>&& Lambda)
{
    FRGPass Pass = {};
    Pass.Name = Name;
    Pass.Reads.assign(Reads.begin(), Reads.end());
    Pass.Queue = Queue;
    Pass.Writes.assign(Writes.begin(), Writes.end());
    Pass.RecordFn = std::move(Lambda);
    RenderGraph.Passes.push_back(Pass);
}

FRGResourceHandle FVulkanRGBuilder::CreateTexture(const FRGTextureDesc& D)
{
    FRGResource R{};
    R.InitialLayout = D.InitialLayout;

    VkImageCreateInfo ImageCreateInfo = {};
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format = D.Fmt;
    ImageCreateInfo.extent = VkExtent3D(D.W, D.H, 1);
    ImageCreateInfo.mipLevels = 1;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.usage = D.UsageFlags;

    VkResult Result = vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &ImageCreateInfo, nullptr, &R.Image);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create transient image.");

    VkMemoryRequirements MemoryRequirements;
    vkGetImageMemoryRequirements(GetVulkanRHIMinimal()->RHIGetVkDevice(), R.Image, &MemoryRequirements);

    VmaVirtualAllocationCreateInfo AllocInfo = {};
    AllocInfo.size = MemoryRequirements.size;
    AllocInfo.alignment = MemoryRequirements.alignment;
    VmaVirtualAllocation Allocation;
    VkDeviceSize Offset;

    Result = vmaVirtualAllocate(RenderGraph.ImagePoolAllocator->VirtualMemory, &AllocInfo, &Allocation, &Offset);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate transient image memory.");

    vkBindImageMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), R.Image, RenderGraph.ImagePoolAllocator->PhysicalMemory, Offset);

    VkImageViewCreateInfo ImageViewCreateInfo = {};
    ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewCreateInfo.image = R.Image;
    ImageViewCreateInfo.format = D.Fmt;
    ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;
    ImageViewCreateInfo.subresourceRange.aspectMask = D.AspectFlags;

    Result = vkCreateImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), &ImageViewCreateInfo, nullptr, &R.ImageView);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");

    RenderGraph.Resources.push_back(R);

    FRGResourceHandle Handle;
    Handle.Index = GCounter++;
    Handle.Type = ERGResourceType::Image;
    return Handle;
}
FRGResourceHandle FVulkanRGBuilder::CreateBuffer(const FRGBufferDesc&  D)
{
    FRGResourceHandle Handle;
    Handle.Index = GCounter++;
    Handle.Type = ERGResourceType::Buffer;
    return Handle;
}

FVulkanRenderGraph::FVulkanRenderGraph()
    : Builder(*this)
{
    ImagePoolAllocator = new FVulkanPoolAllocator();

    CreatePoolAllocator(*ImagePoolAllocator, EPoolAllocatorType::Image, 128 * 1024 * 1024);
}

FVulkanRenderGraph::~FVulkanRenderGraph()
{
    DestroyPoolAllocator(*ImagePoolAllocator);

    delete ImagePoolAllocator;

    for (FRGResource& Resource : Resources)
    {
        vkDestroyImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), Resource.Image, nullptr);
        vkDestroyImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), Resource.ImageView, nullptr);
    }
}

FVulkanRGBuilder &FVulkanRenderGraph::GetMutableBuilder()
{
    return Builder;
}

void FVulkanRenderGraph::Begin()
{
//    Ctx.AddWaitSemaphore(Viewport->GetImageAcquiredSemaphore());
}

void FVulkanRenderGraph::End()
{
//    Ctx.AddSignalSemaphore(Viewport->GetRenderFinishedSemaphore());
    for (FRGResource resource : Resources)
    {
        vkDestroyImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), resource.Image, nullptr);
        vkDestroyImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), resource.ImageView, nullptr);
    }

    Passes.clear();
    Resources.clear();
    CompiledPayloads.clear();

    ResetPoolAllocator(*ImagePoolAllocator);

    GCounter = 0;
}

void FVulkanRenderGraph::Compile()
{
    CompiledPayloads.clear();
    CompiledPayloads.reserve(Passes.size());

    //------------------------------------------------------------
    // State we track while walking the passes
    //------------------------------------------------------------
    struct FImgState
    {
        VkImageLayout     CurrentLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
        EVulkanQueueType  CurrentQueue        = EVulkanQueueType::Graphics;
        int32_t           LastPayloadIndex    = -1;   // where it was last written/read
        bool              FirstUse            = true;
    };
    std::unordered_map<uint32_t, FImgState> ImageStates;   // key = resource index

    //------------------------------------------------------------
    // For now: one payload == one pass
    //------------------------------------------------------------
    for (uint32_t PassIdx = 0; PassIdx < Passes.size(); ++PassIdx)
    {
        const FRGPass& P = Passes[PassIdx];

        FRGCompiledPayload Payload;
        Payload.PassIndices.push_back(PassIdx);
        Payload.Queue        = P.Queue;

        //------------------------------------------------------------------
        // Helper lambda that processes a single Read or Write declaration
        //------------------------------------------------------------------
        auto HandleAccess = [&](const FRGResourceAccess& A, bool bIsWrite)
        {
            if (A.Resource.Type != ERGResourceType::Image)
                return;   // buffer support can be added later

            auto& ImgState = ImageStates[A.Resource.Index];
            const VkImage imgHandle = Resources[A.Resource.Index].Image;

            if (ImgState.FirstUse && ImgState.CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                ImgState.CurrentLayout = Resources[A.Resource.Index].InitialLayout;
            }


            if (ImgState.FirstUse)
            {
                if (ImgState.CurrentLayout != A.Layout)
                {
                    FRGBarrier B;
                    B.Image      = imgHandle;
                    B.OldLayout  = ImgState.CurrentLayout;   // UNDEFINED
                    B.NewLayout  = A.Layout;
                    B.SrcStage   = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;      // nothing to wait for
                    B.DstStage   = A.StageMask;                            // what this pass needs
                    Payload.BeginBarriers.push_back(B);
                }

                ImgState.CurrentLayout    = A.Layout;
                ImgState.CurrentQueue     = P.Queue;
                ImgState.LastPayloadIndex = static_cast<int32_t>(CompiledPayloads.size());
                ImgState.FirstUse         = false;
                return;                     // DONE – no “diff” logic needed
            }

            //-----------------------------------------------------------
            // If this is NOT the first use, check whether we need:
            //  * a layout transition             (layout differs)
            //  * a queue ownership transfer      (queue differs)
            //-----------------------------------------------------------
            if (!ImgState.FirstUse)
            {
                const bool LayoutDiff = (ImgState.CurrentLayout != A.Layout);
                const bool QueueDiff  = (ImgState.CurrentQueue  != P.Queue);

                if (LayoutDiff || QueueDiff)
                {
                    FRGBarrier B;
                    B.Image      = imgHandle;
                    B.OldLayout  = ImgState.CurrentLayout;
                    B.NewLayout  = A.Layout;
                    B.SrcStage   = ImgState.CurrentQueue == P.Queue
                                   ? A.StageMask        // same queue
                                   : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // ownership xfer
                    B.DstStage   = A.StageMask;

                    //------------------------------------------------------------------
                    // Decide where to emit the barrier:
                    //   * same payload  ->   begin-barrier inside this payload
                    //   * different payloads -> end-barrier of producer payload
                    //------------------------------------------------------------------
                    if (ImgState.LastPayloadIndex == static_cast<int32_t>(CompiledPayloads.size()))
                    {
                        Payload.BeginBarriers.push_back(B);
                    }
                    else
                    {
                        CompiledPayloads[ImgState.LastPayloadIndex].EndBarriers.push_back(B);

                        // Queue ownership transfer ⇒ semaphore
                        if (QueueDiff)
                        {
                            // NOTE: You probably have a semaphore pool already.
                            //       For now, pretend we fetch a binary semaphore:
                            // Create a new binary semaphore
                            VkSemaphoreCreateInfo CI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
                            VkSemaphore Sem;
                            vkCreateSemaphore(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CI, nullptr, &Sem);

//                            VkSemaphore xferSem = AcquireTempSemaphore();
                            VkSemaphore xferSem = Sem;

                            CompiledPayloads[ImgState.LastPayloadIndex].Signal.push_back(xferSem);
                            Payload.Wait.push_back(xferSem);
                        }
                    }
                }
            }

            //-----------------------------------------------------------
            // Update state for next pass that touches this image
            //-----------------------------------------------------------
            ImgState.CurrentLayout    = A.Layout;
            ImgState.CurrentQueue     = P.Queue;
            ImgState.LastPayloadIndex = static_cast<int32_t>(CompiledPayloads.size());
            ImgState.FirstUse         = false;
        };

        for (const FRGResourceAccess& R : P.Reads)  HandleAccess(R, /*isWrite=*/false);
        for (const FRGResourceAccess& W : P.Writes) HandleAccess(W, /*isWrite=*/true);

        CompiledPayloads.push_back(std::move(Payload));
    }
}

void FVulkanRenderGraph::Execute(FVulkanCommandBufferContext& FrameCtx)
{
    for (FRGCompiledPayload Plan : CompiledPayloads)
    {
        if (!Plan.Wait.empty())
        {
            FrameCtx.AddWaitSemaphore(std::span<VkSemaphore>(Plan.Wait.data(), Plan.Wait.size()));
        }

        FVulkanCommandBuffer* CB = FrameCtx.GetCurrentCommandBuffer();
        FVulkanCommandListContext CmdCtx(CB->GetHandle());
        FRHICommandList CmdList(CmdCtx);

        for (const FRGBarrier& B : Plan.BeginBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        for (uint32_t Index = 0; Index < Plan.PassIndices.size(); Index++)
        {
            Passes[Plan.PassIndices[Index]].RecordFn(CmdList);
        }

        for (const FRGBarrier& B : Plan.EndBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        if (!Plan.Signal.empty())
        {
            FrameCtx.AddSignalSemaphore(std::span<VkSemaphore>(Plan.Signal.data(), Plan.Signal.size()));
        }

        FRHICommandListExecutor::Execute(CmdCtx, CmdList.Flush());
    }

    // todo: calculate vma statistics at the end of the frame.
}
