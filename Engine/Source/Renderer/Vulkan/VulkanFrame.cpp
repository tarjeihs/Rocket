#include "EnginePCH.h"
#include "VulkanFrame.h"

#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Types/SharedPtr.h"
#include "Types/UniquePtr.h"
#include <vulkan/vulkan_core.h>

void PVulkanFrame::CreateFrame()
{
	CommandPool = new PVulkanCommandPool();
	CommandPool->Create(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	CommandBuffer = new PVulkanCommandBuffer();
	CommandBuffer->Create(CommandPool);

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

	TArray<FVkDescriptorPoolRatio> PoolRatio = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 65536 },
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65536 },
		{VK_DESCRIPTOR_TYPE_SAMPLER, 65536 },
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 65536 },
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
	DescriptorPoolCreateInfo.PoolRatios = PoolRatio;
	DescriptorPoolCreateInfo.MaxSetCount = 1;
	DescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	
	TSharedPtr<FVkDescriptorPool> DescriptorPool = MakeShared<FVkDescriptorPool>();
	DescriptorPool->Initialize(DescriptorPoolCreateInfo);

	TArray<FVkDescriptor> DescriptorStorage = { { EVkDescriptorType::Storage, 65536} };
	TArray<FVkDescriptor> DescriptorStorageImage = { { EVkDescriptorType::StorageImage, 65536} };
	TArray<FVkDescriptor> DescriptorSampler = { { EVkDescriptorType::Sampler, 65536} };
	TArray<FVkDescriptor> DescriptorSamplerImage = { { EVkDescriptorType::SamplerImage, 65536} };

	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutStorageCreateInfo = { .Descriptors = DescriptorStorage };
	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutStorageImageCreateInfo = { .Descriptors = DescriptorStorageImage };
	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutSamplerCreateInfo = { .Descriptors = DescriptorSampler };
	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutSamplerImageCreateInfo = { .Descriptors = DescriptorSamplerImage };

	TSharedPtr<FVkDescriptorSetLayout> BindlessDescriptorSetLayoutStorage = MakeShared<FVkDescriptorSetLayout>();
	TSharedPtr<FVkDescriptorSetLayout> BindlessDescriptorSetLayoutStorageImage = MakeShared<FVkDescriptorSetLayout>();
	TSharedPtr<FVkDescriptorSetLayout> BindlessDescriptorSetLayoutSampler = MakeShared<FVkDescriptorSetLayout>();
	TSharedPtr<FVkDescriptorSetLayout> BindlessDescriptorSetLayoutSamplerImage = MakeShared<FVkDescriptorSetLayout>();

	BindlessDescriptorSetLayoutStorage->Initialize(DescriptorSetLayoutStorageCreateInfo);
	BindlessDescriptorSetLayoutStorageImage->Initialize(DescriptorSetLayoutStorageCreateInfo);
	BindlessDescriptorSetLayoutSampler->Initialize(DescriptorSetLayoutStorageCreateInfo);
	BindlessDescriptorSetLayoutSamplerImage->Initialize(DescriptorSetLayoutStorageCreateInfo);

	FVkDescriptorSetCreateInfo BindlessDescriptorSetStorageCreateInfo;
	BindlessDescriptorSetStorageCreateInfo.DescriptorPool = DescriptorPool;
	BindlessDescriptorSetStorageCreateInfo.DescriptorSetLayout = BindlessDescriptorSetLayoutStorage;

	FVkDescriptorSetCreateInfo BindlessDescriptorSetStorageImageCreateInfo;
	BindlessDescriptorSetStorageImageCreateInfo.DescriptorPool = DescriptorPool;
	BindlessDescriptorSetStorageImageCreateInfo.DescriptorSetLayout = BindlessDescriptorSetLayoutStorageImage;

	FVkDescriptorSetCreateInfo BindlessDescriptorSetSamplerCreateInfo;
	BindlessDescriptorSetSamplerCreateInfo.DescriptorPool = DescriptorPool;
	BindlessDescriptorSetSamplerCreateInfo.DescriptorSetLayout = BindlessDescriptorSetLayoutSampler;

	FVkDescriptorSetCreateInfo BindlessDescriptorSetSamplerImageCreateInfo;
	BindlessDescriptorSetSamplerImageCreateInfo.DescriptorPool = DescriptorPool;
	BindlessDescriptorSetSamplerImageCreateInfo.DescriptorSetLayout = BindlessDescriptorSetLayoutSamplerImage;

	BindlessDescriptorSetStorageBuffer = MakeShared<FVkDescriptorSet>();
	BindlessDescriptorSetStorageImage = MakeShared<FVkDescriptorSet>();
	BindlessDescriptorSetSampler = MakeShared<FVkDescriptorSet>();
	BindlessDescriptorSetSamplerImage = MakeShared<FVkDescriptorSet>();
	
	BindlessDescriptorSetStorageBuffer->Initialize(BindlessDescriptorSetStorageCreateInfo);
	BindlessDescriptorSetStorageImage->Initialize(BindlessDescriptorSetStorageImageCreateInfo);
	BindlessDescriptorSetSampler->Initialize(BindlessDescriptorSetSamplerCreateInfo);
	BindlessDescriptorSetSamplerImage->Initialize(BindlessDescriptorSetSamplerImageCreateInfo);
}

void PVulkanFrame::DestroyFrame()
{
	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), RenderSemaphore, nullptr);
	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), SwapchainSemaphore, nullptr);

	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), RenderFence, nullptr);
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool->GetVkCommandPool(), nullptr);
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

	PipelineStateData = new PVulkanPipelineStateData();
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