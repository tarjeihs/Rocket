#include "EnginePCH.h"
#include "VulkanFrame.h"

#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanFrame::CreateFrame()
{
	CommandPool = new PVulkanCommandPool();
	CommandPool->Create(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	CommandBuffer = new PVulkanCommandBuffer();
	CommandBuffer->Create(CommandPool);

	Memory = new PVulkanMemory();
	Memory->Init();

	VkFenceCreateInfo FenceCreateInfo{};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.pNext = nullptr;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.pNext = nullptr;
	SemaphoreCreateInfo.flags = 0;

	VkResult Result = vkCreateFence(GetRHI()->GetDevice()->GetVkDevice(), &FenceCreateInfo, nullptr, &RenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create render fence.");

	Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &SwapchainSemaphore);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain semaphore.");

	Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &RenderSemaphore);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create render semaphore.");
}

void PVulkanFrame::DestroyFrame()
{
	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), RenderSemaphore, nullptr);
	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), SwapchainSemaphore, nullptr);

	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), RenderFence, nullptr);
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool->GetVkCommandPool(), nullptr);

	Memory->Shutdown();
	delete Memory;
}

void PVulkanFrame::BeginFrame()
{
	PROFILE_FUNC_SCOPE("PVulkanFrame::BeginFrame")

	vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence, VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(GetRHI()->GetDevice()->GetVkDevice(), GetRHI()->GetSceneRenderer()->GetSwapchain()->GetVkSwapchain(), UINT64_MAX, SwapchainSemaphore, nullptr, &TransientFrameData.NextImageIndex);
	vkResetFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence);
	CommandBuffer->ResetCommandBuffer();
	CommandBuffer->BeginCommandBuffer();
}

void PVulkanFrame::EndFrame()
{
	PROFILE_FUNC_SCOPE("PVulkanFrame::EndFrame")

	CommandBuffer->EndCommandBuffer();

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.commandBuffer = CommandBuffer->GetVkCommandBuffer();

	VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
	WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	WaitSemaphoreSubmitInfo.semaphore = SwapchainSemaphore;
	WaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

	VkSemaphoreSubmitInfo SignalSemaphoreSubmitInfo = {};
	SignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	SignalSemaphoreSubmitInfo.semaphore = RenderSemaphore;
	SignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

	VkSubmitInfo2 SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	SubmitInfo.waitSemaphoreInfoCount = 1;
	SubmitInfo.pWaitSemaphoreInfos = &WaitSemaphoreSubmitInfo;
	SubmitInfo.signalSemaphoreInfoCount = 1;
	SubmitInfo.pSignalSemaphoreInfos = &SignalSemaphoreSubmitInfo;
	SubmitInfo.commandBufferInfoCount = 1;
	SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

	VkResult Result = vkQueueSubmit2(GetRHI()->GetDevice()->GetGraphicsQueue(), 1, &SubmitInfo, RenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");

	VkSwapchainKHR SwapchainPointer = GetRHI()->GetSceneRenderer()->GetSwapchain()->GetVkSwapchain();
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &SwapchainPointer;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &TransientFrameData.NextImageIndex;

	vkQueuePresentKHR(GetRHI()->GetDevice()->GetGraphicsQueue(), &presentInfo);
}

PVulkanCommandPool* PVulkanFrame::GetCommandPool() const
{
	return CommandPool;
}

PVulkanCommandBuffer* PVulkanFrame::GetCommandBuffer() const
{
	return CommandBuffer;
}

PVulkanMemory* PVulkanFrame::GetMemory() const
{
	return Memory;
}

VkSemaphore PVulkanFrame::GetSwapchainSemaphore() const
{
	return SwapchainSemaphore;
}

VkSemaphore PVulkanFrame::GetRenderSemaphore() const
{
	return RenderSemaphore;
}

VkFence PVulkanFrame::GetRenderFence() const
{
	return RenderFence;
}

FTransientFrameData& PVulkanFrame::GetTransientFrameData()
{
	return TransientFrameData;
}

void PVulkanFramePool::CreateFramePool()
{
	for (size_t Index = 0; Index < PoolSize; ++Index)
	{
		PVulkanFrame* Frame = new PVulkanFrame();
		Frame->CreateFrame();
		Pool.push_back(Frame);
	}
}

void PVulkanFramePool::FreeFramePool()
{
	for (size_t Index = 0; Index < PoolSize; ++Index)
	{
		Pool[Index]->DestroyFrame();
		delete Pool[Index];
		Pool[Index] = nullptr;
	}
}

PVulkanFrame* PVulkanFramePool::GetCurrentFrame() const
{
	return Pool[FrameIndex % PoolSize];
}

size_t PVulkanFramePool::GetCurrentFrameIndex() const
{
	return FrameIndex % PoolSize;
}