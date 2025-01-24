#include "EnginePCH.h"
#include "VkRenderer.h"

#include "Renderer/Common/Overlay.h"
#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VkScriptableRendererPipeline.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VkOverlay.h"
#include "Renderer/Common/Memory.h"
#include "Types/UniquePtr.h"
#include "Utils/Profiler.h"
#include "VkMemory.h"

void FVkRenderer::Init()
{
	Swapchain = MakeUnique<PVulkanSwapchain>();
	Swapchain->Init();

	GMemory = new FVkMemory();
	GMemory->Initialize();

	for (SizeType Frame = 0; Frame < CONCURRENT_FRAME_COUNT; ++Frame)
	{		
		CommandPool[Frame] = MakeUnique<FVkCommandPool>();
		CommandPool[Frame]->Initialize(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		CommandBuffer[Frame] = MakeUnique<FVkCommandBuffer>();
		CommandBuffer[Frame]->Initialize(CommandPool[Frame].Get());

		VkFenceCreateInfo FenceCreateInfo = {};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.pNext = VK_NULL_HANDLE;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreCreateInfo.pNext = VK_NULL_HANDLE;
		SemaphoreCreateInfo.flags = 0;

		VkResult Result = vkCreateFence(GetRHI()->GetDevice()->GetVkDevice(), &FenceCreateInfo, nullptr, &RenderFence[Frame]);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create render fence.");

		Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &SwapchainSemaphore[Frame]);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain semaphore.");

		Result = vkCreateSemaphore(GetRHI()->GetDevice()->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &RenderSemaphore[Frame]);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create render semaphore.");
	}

	ImmediateCommandPool = MakeUnique<FVkCommandPool>();
	ImmediateCommandPool->Initialize(GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	ImmediateCommandBuffer = MakeUnique<FVkCommandBuffer>();
	ImmediateCommandBuffer->Initialize(ImmediateCommandPool.Get());

	VkFenceCreateInfo ImmediateFenceCreateInfo = {};
	ImmediateFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ImmediateFenceCreateInfo.pNext = VK_NULL_HANDLE;
	ImmediateFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult Result = vkCreateFence(GetRHI()->GetDevice()->GetVkDevice(), &ImmediateFenceCreateInfo, nullptr, &ImmediateRenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create immediate render fence.");

	//PipelineLayout												= new FVkPipelineLayout();
	//DescriptorSet 												= new FVkDescriptorSet();
	//DescriptorSetLayout 										= new FVkDescriptorSetLayout();
	//DescriptorPool 												= new FVkDescriptorPool();
	//
	//FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo =
	//{{
	//	{ EVkDescriptorType::StructuredBuffer, 	16 },
	//	{ EVkDescriptorType::RWTexture2D, 		16 },
	//	{ EVkDescriptorType::Texture2D, 		16 },
	//}};
	//
	//FVkDescriptorSetCreateInfo DescriptorSetCreateInfo =
	//{
	//	DescriptorPool,
	//	DescriptorSetLayout
	//};
	//
	//FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = 
	//{
	//	{
    //		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			16.0f },
	//		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 			16.0f },
	//		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 	16.0f },
	//	},
	//	1,
	//	VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
	//};
	//
	//FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo =
	//{
	//	{ 
	//		DescriptorSetLayout
	//	}
	//};
	//
	//DescriptorPool->Initialize(DescriptorPoolCreateInfo);
	//DescriptorSetLayout->Initialize(DescriptorSetLayoutCreateInfo);
	//DescriptorSet->Initialize(DescriptorSetCreateInfo);
	//PipelineLayout->Initialize(PipelineLayoutCreateInfo);

	//FImageCreateInfo IntermediateColorAttachmentCreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::Storage | EImageUsage::TransferDst | EImageUsage::TransferSrc | EImageUsage::ColorAttachment,
	//	EImageAspect::Color,
	//	{1280,720},
	//	EImageFormat::R16G16B16A16_SFLOAT
	//};
	//
	//FImageCreateInfo PresentColorAttachmentCreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::TransferDst | EImageUsage::TransferSrc | EImageUsage::ColorAttachment,
	//	EImageAspect::Color,
	//	{1280,720},
	//	EImageFormat::A2B10G10R10_UNORM_PACK32
	//};
	//
	//FImageCreateInfo DepthAttachment16CreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::DepthStencilAttachment,
	//	EImageAspect::Depth,
	//	{1280,720},
	//	EImageFormat::D32_SFLOAT
	//};
	//
	//IntermediateColorAttachment = MakeUnique<FVkImage>();
	//IntermediateColorAttachment->Initialize(IntermediateColorAttachmentCreateInfo);
	//
	//PresentColorAttachment = MakeUnique<FVkImage>();
	//PresentColorAttachment->Initialize(PresentColorAttachmentCreateInfo);
	//
	//DepthAttachmentD32 = MakeUnique<FVkImage>();
	//DepthAttachmentD32->Initialize(DepthAttachment16CreateInfo);

	//MeshAllocator = MakeUnique<FVkStaticMeshBuffer>();
	//MeshAllocator->Initialize();
	//
	//GOverlay = new FVkOverlay();
	//GOverlay->Init();
}

void FVkRenderer::Shutdown()
{
	//GOverlay->Shutdown();
	//delete GOverlay;

    Swapchain->Shutdown();
	GMemory->Shutdown();

	//IntermediateColorAttachment->Shutdown();
	//PresentColorAttachment->Shutdown();
	//DepthAttachmentD32->Shutdown();

	//DescriptorSet->Shutdown();
	//DescriptorSetLayout->Shutdown();
	//DescriptorPool->Shutdown();

    for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
    {
        vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), RenderSemaphore[Index], nullptr);
    	vkDestroySemaphore(GetRHI()->GetDevice()->GetVkDevice(), SwapchainSemaphore[Index], nullptr);
    	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), RenderFence[Index], nullptr);
    	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool[Index]->GetVkCommandPool(), nullptr);
    }

	vkDestroyFence(GetRHI()->GetDevice()->GetVkDevice(), ImmediateRenderFence, nullptr);
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), ImmediateCommandPool->GetVkCommandPool(), nullptr);

	//MeshAllocator->Shutdown();
}

void FVkRenderer::Render()
{
    PROFILE_FUNC_SCOPE("FVkRenderer::Render")

    BeginFrame();

    //IntermediateColorAttachment->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//PresentColorAttachment->TransitionImageLayout(GetCommandBuffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //DepthAttachmentD32->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Memory->Info.PipelineLayout->Info.Handle, 0, 1, &Memory->Info.DescriptorSet->Info.Handle, 0, 0);
	//vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Memory->Info.PipelineLayout->Info.Handle, 0, 1, &Memory->Info.DescriptorSet->Info.Handle, 0, 0);

    //Bind(); // Bind descriptor sets, update buffers, etc...
	
	GMemory->Execute();

	for (const auto& Pipeline : ScriptableRendererPipelineData)
	{
		Pipeline->Execute();
	}

    //IntermediateColorAttachment->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    //PresentColorAttachment->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	
	//GOverlay->Execute();
    
	Swapchain->Info.Backbuffer[GetNextImageIndex()]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //PresentColorAttachment->CopyImageRegion(CommandBuffer[FrameIndex].Get(), Swapchain->Info.Backbuffer[NextImageIndex[FrameIndex]]->Info.ImageHandle, PresentColorAttachment->Info.Extent, Swapchain->Info.SwapchainImageExtent);
	Swapchain->Info.Backbuffer[GetNextImageIndex()]->TransitionImageLayout(CommandBuffer[FrameIndex].Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    EndFrame();

    FrameIndex = (FrameIndex + 1) % CONCURRENT_FRAME_COUNT;
}

void FVkRenderer::Resize()
{
	PROFILE_FUNC_SCOPE("FVkRenderer::Resize")

	Swapchain->Shutdown();
	Swapchain->Init();

	//FImageCreateInfo IntermediateColorAttachmentCreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::Storage | EImageUsage::TransferDst | EImageUsage::TransferSrc | EImageUsage::ColorAttachment,
	//	EImageAspect::Color,
	//	{1280,720},
	//	EImageFormat::R16G16B16A16_SFLOAT
	//};
	//
	//FImageCreateInfo PresentColorAttachmentCreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::TransferDst | EImageUsage::TransferSrc | EImageUsage::ColorAttachment,
	//	EImageAspect::Color,
	//	{1280,720},
	//	//EImageFormat::R16G16B16A16_SFLOAT
	//};
	//
	//FImageCreateInfo DepthAttachment16CreateInfo =
	//{
	//	EImageLayout::Undefined,
	//	EImageUsage::DepthStencilAttachment,
	//	EImageAspect::Depth,
	//	{1280,720},
	//	EImageFormat::D32_SFLOAT
	//};
	//
	//IntermediateColorAttachment->Shutdown();
	//IntermediateColorAttachment->Initialize(IntermediateColorAttachmentCreateInfo);
	//
	//PresentColorAttachment->Shutdown();
	//PresentColorAttachment->Initialize(PresentColorAttachmentCreateInfo);
	//
	//DepthAttachmentD32->Shutdown();
	//DepthAttachmentD32->Initialize(DepthAttachment16CreateInfo);
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

void FVkRenderer::ImmediateSubmit(std::function<void(FVkCommandBuffer*)>&& Func)
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