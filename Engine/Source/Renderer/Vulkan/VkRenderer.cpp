#include "EnginePCH.h"
#include "VkRenderer.h"

#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Types/UniquePtr.h"
#include "Utils/Profiler.h"

void FVkRenderer::Init()
{
	Swapchain = MakeUnique<PVulkanSwapchain>();
    RenderGraph = MakeUnique<PVulkanRenderGraph>();

	Swapchain->Init();

	FVkImageCreateInfo ColorAttachment16CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_R16G16B16A16_SFLOAT
	};
	
	FVkImageCreateInfo ColorAttachment8CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_B8G8R8A8_SRGB
	};

	FVkImageCreateInfo DepthAttachment16CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_D32_SFLOAT
	};

	ColorAttachment16 = MakeUnique<FVkImage>();
	ColorAttachment16->Initialize(ColorAttachment16CreateInfo);

	ColorAttachment8 = MakeUnique<FVkImage>();
	ColorAttachment8->Initialize(ColorAttachment8CreateInfo);

	DepthAttachmentD32 = MakeUnique<FVkImage>();
	DepthAttachmentD32->Initialize(DepthAttachment16CreateInfo);

	for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
    {
        CommandPool[Index] = MakeUnique<PVulkanCommandPool>();
	    CommandPool[Index]->Create(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	    CommandBuffer[Index] = MakeUnique<PVulkanCommandBuffer>();
	    CommandBuffer[Index]->Create(CommandPool[Index].Get());

        VkFenceCreateInfo FenceCreateInfo = {};
	    FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	    FenceCreateInfo.pNext = VK_NULL_HANDLE;
	    FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	    SemaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	    SemaphoreCreateInfo.flags = 0;

        VkResult Result = vkCreateFence(GetRHI()->GetDevice()->GetVkDevice(), &FenceCreateInfo, nullptr, &RenderFence[Index]);
	    RK_ASSERT(Result == VK_SUCCESS, "Failed to create render fence.");

	    Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &SwapchainSemaphore[Index]);
	    RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain semaphore.");

	    Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &RenderSemaphore[Index]);
	    RK_ASSERT(Result == VK_SUCCESS, "Failed to create render semaphore.");
    }

	ImmediateCommandPool = MakeUnique<PVulkanCommandPool>();
	ImmediateCommandPool->Create(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	ImmediateCommandBuffer = MakeUnique<PVulkanCommandBuffer>();
	ImmediateCommandBuffer->Create(ImmediateCommandPool.Get());

	VkFenceCreateInfo ImmediateFenceCreateInfo = {};
	ImmediateFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ImmediateFenceCreateInfo.pNext = VK_NULL_HANDLE;
	ImmediateFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult Result = vkCreateFence(GetRHI()->GetDevice()->GetVkDevice(), &ImmediateFenceCreateInfo, nullptr, &ImmediateRenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create immediate render fence.");

	MeshAllocator = MakeUnique<FVkMeshAllocator>();
	MeshAllocator->Initialize();
}

void FVkRenderer::Shutdown()
{
    Swapchain->Shutdown();

	ColorAttachment16->Shutdown();
	ColorAttachment8->Shutdown();
	DepthAttachmentD32->Shutdown();

    for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
    {
        vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), RenderSemaphore[Index], nullptr);
    	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), SwapchainSemaphore[Index], nullptr);
    	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), RenderFence[Index], nullptr);
    	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool[Index]->GetVkCommandPool(), nullptr);
    }

	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), ImmediateRenderFence, nullptr);
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), ImmediateCommandPool->GetVkCommandPool(), nullptr);

	MeshAllocator->Shutdown();
}

void FVkRenderer::Render()
{
    PROFILE_FUNC_SCOPE("FVkRenderer::Render")

    BeginFrame();

    ColorAttachment16->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	ColorAttachment8->TransitionImageLayout(GetCommandBuffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    DepthAttachmentD32->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    Bind(); // Bind descriptor sets, update buffers, etc...
	
	for (const auto& Pipeline : ScriptableRendererPipelineData)
	{
		Pipeline->Execute();
	}

    ColorAttachment16->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    ColorAttachment8->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    
	Swapchain->Info.Backbuffer[NextImageIndex[FrameIndex]]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ColorAttachment8->CopyImageRegion(CommandBuffer[FrameIndex].Get(), Swapchain->Info.Backbuffer[NextImageIndex[FrameIndex]]->Info.ImageHandle, ColorAttachment8->Info.Extent, Swapchain->Info.SwapchainImageExtent);
	Swapchain->Info.Backbuffer[NextImageIndex[FrameIndex]]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    EndFrame();

    FrameIndex = (FrameIndex + 1) % CONCURRENT_FRAME_COUNT;
}

void FVkRenderer::Resize()
{
	PROFILE_FUNC_SCOPE("FVkRenderer::Resize")

	Swapchain->Shutdown();
	Swapchain->Init();

	FVkImageCreateInfo ColorAttachment16CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		Swapchain->Info.SwapchainImageExtent,
		VK_FORMAT_R16G16B16A16_SFLOAT
	};
	
	FVkImageCreateInfo ColorAttachment8CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		Swapchain->Info.SwapchainImageExtent,
		VK_FORMAT_B8G8R8A8_SRGB
	};

	FVkImageCreateInfo DepthAttachmentCreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		Swapchain->Info.SwapchainImageExtent,
		VK_FORMAT_D32_SFLOAT
	};

	ColorAttachment16->Shutdown();
	ColorAttachment16->Initialize(ColorAttachment16CreateInfo);

	ColorAttachment8->Shutdown();
	ColorAttachment8->Initialize(ColorAttachment8CreateInfo);

	DepthAttachmentD32->Shutdown();
	DepthAttachmentD32->Initialize(DepthAttachmentCreateInfo);
}

void FVkRenderer::BeginFrame()
{
    PROFILE_FUNC_SCOPE("FVkRenderer::BeginFrame") 

	vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence[FrameIndex], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(GetRHI()->GetDevice()->GetVkDevice(), Swapchain->Info.SwapchainKHR, UINT64_MAX, SwapchainSemaphore[FrameIndex], VK_NULL_HANDLE, &NextImageIndex[FrameIndex]);
	vkResetFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence[FrameIndex]);
	
	CommandBuffer[FrameIndex]->ResetCommandBuffer();
	CommandBuffer[FrameIndex]->BeginCommandBuffer();
}

void FVkRenderer::EndFrame()
{
	PROFILE_FUNC_SCOPE("FVkRenderer::EndFrame")

	CommandBuffer[FrameIndex]->EndCommandBuffer();

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.commandBuffer = CommandBuffer[FrameIndex]->GetVkCommandBuffer();

	VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
	WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	WaitSemaphoreSubmitInfo.semaphore = SwapchainSemaphore[FrameIndex];
	WaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

	VkSemaphoreSubmitInfo SignalSemaphoreSubmitInfo = {};
	SignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	SignalSemaphoreSubmitInfo.semaphore = RenderSemaphore[FrameIndex];
	SignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

	VkSubmitInfo2 SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	SubmitInfo.waitSemaphoreInfoCount = 1;
	SubmitInfo.pWaitSemaphoreInfos = &WaitSemaphoreSubmitInfo;
	SubmitInfo.signalSemaphoreInfoCount = 1;
	SubmitInfo.pSignalSemaphoreInfos = &SignalSemaphoreSubmitInfo;
	SubmitInfo.commandBufferInfoCount = 1;
	SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

	VkResult Result = vkQueueSubmit2(GetRHI()->GetDevice()->GetGraphicsQueue(), 1, &SubmitInfo, RenderFence[FrameIndex]);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");

	VkSwapchainKHR SwapchainPointer = Swapchain->Info.SwapchainKHR;
	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = VK_NULL_HANDLE;
	PresentInfo.pSwapchains = &SwapchainPointer;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pWaitSemaphores = &RenderSemaphore[FrameIndex];
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pImageIndices = &NextImageIndex[FrameIndex];

	Result = vkQueuePresentKHR(GetRHI()->GetDevice()->GetGraphicsQueue(), &PresentInfo);
}

void FVkRenderer::ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func)
{
	PROFILE_FUNC_SCOPE("FVkRenderer::ImmediateSubmit")

	VkResult Result = vkResetFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &ImmediateRenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset fence.");

	Result = vkResetCommandBuffer(ImmediateCommandBuffer->GetVkCommandBuffer(), 0);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");
	
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.pNext = nullptr;
	CommandBufferBeginInfo.pInheritanceInfo = nullptr;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	Result = vkBeginCommandBuffer(ImmediateCommandBuffer->GetVkCommandBuffer(), &CommandBufferBeginInfo);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to begin command buffer.");

	Func(ImmediateCommandBuffer.Get());

	Result = vkEndCommandBuffer(ImmediateCommandBuffer->GetVkCommandBuffer());
	RK_ASSERT(Result == VK_SUCCESS, "Failed to end command buffer..");

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo{};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.pNext = nullptr;
	CommandBufferSubmitInfo.commandBuffer = ImmediateCommandBuffer->GetVkCommandBuffer();
	CommandBufferSubmitInfo.deviceMask = 0;

	VkSubmitInfo2 SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	SubmitInfo.pNext = nullptr;
	SubmitInfo.waitSemaphoreInfoCount = 0;
	SubmitInfo.pWaitSemaphoreInfos = nullptr;
	SubmitInfo.signalSemaphoreInfoCount = 0;
	SubmitInfo.pSignalSemaphoreInfos = nullptr;
	SubmitInfo.commandBufferInfoCount = 1;
	SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

	// Submit the command buffer to the graphics queue for execution.
	// The RenderFence will now block until all graphics commands have completed.
	Result = vkQueueSubmit2(GetRHI()->GetDevice()->GetGraphicsQueue(), 1, &SubmitInfo, ImmediateRenderFence);
	Result = vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &ImmediateRenderFence, true, UINT64_MAX);
}