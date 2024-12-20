#include "EnginePCH.h"
#include "VkRenderer.h"

#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VkSceneBuffer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Types/UniquePtr.h"
#include "Utils/Profiler.h"

void FVkRenderer::Init()
{
	Swapchain = MakeUnique<PVulkanSwapchain>();
    RenderGraph = MakeUnique<PVulkanRenderGraph>();

	Swapchain->Init();

	ColorAttachmentImage = MakeUnique<FVkImage>();
	ColorAttachmentImage->Init(Swapchain->GetVkExtent(), VK_FORMAT_R16G16B16A16_SFLOAT);
	ColorAttachmentImage->CreateImage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	ColorAttachmentImage->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	DepthAttachmentImage = MakeUnique<FVkImage>();
	DepthAttachmentImage->Init(Swapchain->GetVkExtent(), VK_FORMAT_D32_SFLOAT);
	DepthAttachmentImage->CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	DepthAttachmentImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    
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

	SceneBuffer = MakeUnique<FVkSceneBuffer>();
	SceneBuffer->Initialize();
}

void FVkRenderer::Shutdown()
{
    Swapchain->Shutdown();

	ColorAttachmentImage->DestroyImage();
	ColorAttachmentImage->DestroyImageView();

	DepthAttachmentImage->DestroyImage();
	DepthAttachmentImage->DestroyImageView();

    for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
    {
        vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), RenderSemaphore[Index], nullptr);
    	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), SwapchainSemaphore[Index], nullptr);
    	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), RenderFence[Index], nullptr);
    	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool[Index]->GetVkCommandPool(), nullptr);
    }

	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), ImmediateRenderFence, nullptr);
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), ImmediateCommandPool->GetVkCommandPool(), nullptr);

	SceneBuffer->Shutdown();
}

void FVkRenderer::Render()
{
	PROFILE_FUNC_SCOPE("FVkRenderer::Render")

	BeginFrame();
	ColorAttachmentImage->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	DepthAttachmentImage->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	RenderGraph->BeginRendering();
	RenderGraph->Execute(CommandBuffer[FrameIndex].Get());
	Bind();
	RenderGraph->EndRendering();
	ColorAttachmentImage->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	
	Swapchain->GetSwapchainImages()[NextImageIndex[FrameIndex]]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	ColorAttachmentImage->CopyImageRegion(CommandBuffer[FrameIndex].Get(), Swapchain->GetSwapchainImages()[NextImageIndex[FrameIndex]]->GetVkImage(), ColorAttachmentImage->GetImageExtent2D(), Swapchain->GetVkExtent());
	Swapchain->GetSwapchainImages()[NextImageIndex[FrameIndex]]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	
	EndFrame();
}

void FVkRenderer::Resize()
{
	PROFILE_FUNC_SCOPE("FVkRenderer::Resize")

	Swapchain->Shutdown();
	Swapchain->Init();

	ColorAttachmentImage->DestroyImage();
	ColorAttachmentImage->DestroyImageView();
	ColorAttachmentImage->Reset();
	ColorAttachmentImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_R16G16B16A16_SFLOAT);
	ColorAttachmentImage->CreateImage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	ColorAttachmentImage->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	DepthAttachmentImage->DestroyImage();
	DepthAttachmentImage->DestroyImageView();
	DepthAttachmentImage->Reset();
	DepthAttachmentImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_D32_SFLOAT);
	DepthAttachmentImage->CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	DepthAttachmentImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);	
}

void FVkRenderer::BeginFrame()
{
    PROFILE_FUNC_SCOPE("FVkRenderer::BeginFrame")

	vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence[FrameIndex], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(GetRHI()->GetDevice()->GetVkDevice(), Swapchain->GetVkSwapchain(), UINT64_MAX, SwapchainSemaphore[FrameIndex], VK_NULL_HANDLE, &NextImageIndex[FrameIndex]);
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

	VkSwapchainKHR SwapchainPointer = Swapchain->GetVkSwapchain();
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

// Set 0 SSBO 
//				0	Global
//				1	Camera
//				2	Material
//				3	Instance

// Set 1 Textures w/ Samplers
//				0	Albedo
//				1	Metallic
//				2 	Normal