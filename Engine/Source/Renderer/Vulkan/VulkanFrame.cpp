#include "EnginePCH.h"
#include "VulkanFrame.h"

#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Types/SharedPtr.h"

TSharedPtr<FVkDescriptorSetLayout> StorageBufferDescriptorSetLayout = MakeShared<FVkDescriptorSetLayout>();

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
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65536 },
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
	DescriptorPoolCreateInfo.PoolRatios = PoolRatio;
	DescriptorPoolCreateInfo.MaxSetCount = 1;
	DescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	
	TSharedPtr<FVkDescriptorPool> DescriptorPool = MakeShared<FVkDescriptorPool>();
	DescriptorPool->Initialize(DescriptorPoolCreateInfo);

	/* Descriptor Set Layout - Storage Buffer */

	FVkBufferCreateInfo StorageBufferCreateInfo;
	StorageBufferCreateInfo.Size = 1024 * 1024 * 10;
	StorageBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	StorageBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_CPU_TO_GPU;

	GlobalStorageBuffer = MakeShared<FVkBuffer>();
	GlobalStorageBuffer->Initialize(StorageBufferCreateInfo);

	CameraStorageBuffer = MakeShared<FVkBuffer>();
	CameraStorageBuffer->Initialize(StorageBufferCreateInfo);

	MaterialStorageBuffer = MakeShared<FVkBuffer>();
	MaterialStorageBuffer->Initialize(StorageBufferCreateInfo);

	ObjectStorageBuffer = MakeShared<FVkBuffer>();
	ObjectStorageBuffer->Initialize(StorageBufferCreateInfo);

	FVkDescriptorSetCreateInfo StorageBufferDescriptorSetCreateInfo;
	StorageBufferDescriptorSetCreateInfo.DescriptorPool = DescriptorPool;
	StorageBufferDescriptorSetCreateInfo.DescriptorSetLayout = StorageBufferDescriptorSetLayout;

	StorageBufferDescriptorSet = MakeShared<FVkDescriptorSet>();	
	StorageBufferDescriptorSet->Initialize(StorageBufferDescriptorSetCreateInfo);
	StorageBufferDescriptorSet->AttachBuffer(STORAGE_BUFFER_DESCRIPTOR_INDEX_GLOBAL, GlobalStorageBuffer);
	StorageBufferDescriptorSet->AttachBuffer(STORAGE_BUFFER_DESCRIPTOR_INDEX_CAMERA, CameraStorageBuffer);
	StorageBufferDescriptorSet->AttachBuffer(STORAGE_BUFFER_DESCRIPTOR_INDEX_MATERIAL, MaterialStorageBuffer);
	StorageBufferDescriptorSet->AttachBuffer(STORAGE_BUFFER_DESCRIPTOR_INDEX_OBJECT, ObjectStorageBuffer);
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
	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = nullptr;
	PresentInfo.pSwapchains = &SwapchainPointer;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pWaitSemaphores = &RenderSemaphore;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pImageIndices = &TransientFrameData.NextImageIndex;

	vkQueuePresentKHR(GetRHI()->GetDevice()->GetGraphicsQueue(), &PresentInfo);
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
	TArray<FVkDescriptor> StorageBufferDescriptors = { 
		{ EVkDescriptorType::Storage, 1 },
		{ EVkDescriptorType::Storage, 1 },
		{ EVkDescriptorType::Storage, 1 },
		{ EVkDescriptorType::Storage, 1 }
	};

	FVkDescriptorSetLayoutCreateInfo StorageBufferDescriptorSetLayoutCreateInfo = { .Descriptors = StorageBufferDescriptors };

	StorageBufferDescriptorSetLayout->Initialize(StorageBufferDescriptorSetLayoutCreateInfo);

	FVkPipelineLayoutCreateInfo GraphicsPipelineLayoutCreateInfo;
	GraphicsPipelineLayoutCreateInfo.DescriptorSetLayouts = {
		StorageBufferDescriptorSetLayout->Info.DescriptorSetLayout
	};

	GraphicsPipelineLayout = MakeShared<FVkPipelineLayout>();
	GraphicsPipelineLayout->Initialize(GraphicsPipelineLayoutCreateInfo);

    FShaderCreateInfo ShaderCreateInfo;
    ShaderCreateInfo.Entrypoint = "main";
    ShaderCreateInfo.Path = "/home/user/Workspace/Rocket/Engine/Shaders/HLSL/Vertex.hlsl";
    ShaderCreateInfo.Name = "Vertex";
    ShaderCreateInfo.Stage = EShaderStage::Vertex;

	FShaderCreateInfo PixelShaderCreateInfo;
    PixelShaderCreateInfo.Entrypoint = "main";
    PixelShaderCreateInfo.Path = "/home/user/Workspace/Rocket/Engine/Shaders/HLSL/Pixel.hlsl";
    PixelShaderCreateInfo.Name = "Pixel";
    PixelShaderCreateInfo.Stage = EShaderStage::Fragment;
    
	TSharedPtr<PVulkanShader> DefaultLitVertexShader = MakeShared<PVulkanShader>();
	DefaultLitVertexShader->CreateShader(ShaderCreateInfo);

	TSharedPtr<PVulkanShader> DefaultLitPixelShader = MakeShared<PVulkanShader>();
	DefaultLitPixelShader->CreateShader(PixelShaderCreateInfo);

	FVkPipelineCreateInfo GraphicsPipelineCreateInfo;
	GraphicsPipelineCreateInfo.PipelineLayout = GraphicsPipelineLayout;
	GraphicsPipelineCreateInfo.Shaders = {DefaultLitVertexShader, DefaultLitPixelShader};

	GraphicsPipeline = MakeShared<FVkPipeline>();
	GraphicsPipeline->Initialize(GraphicsPipelineCreateInfo);
	
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