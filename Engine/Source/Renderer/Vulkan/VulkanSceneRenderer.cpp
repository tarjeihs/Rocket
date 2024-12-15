#include "EnginePCH.h"
#include "VulkanSceneRenderer.h"

#include "Format/GLTF.h"
#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanOverlay.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Scene/Scene.h"
#include "Types/SharedPtr.h"

// 3 swapchain images, 2 frames.
// 1 frame is currently being processed by CPU, 1 is being drawn by GPU, hence no need for a third frame.
static constexpr size_t DeferredFrameCount = 2;
static constexpr size_t ImmediateFrameCount = 1;

TSharedPtr<FVkMeshBuffer> MeshBuffer;

void PVulkanSceneRenderer::Init()
{
	Allocator = new PVulkanAllocator();
	Swapchain = new PVulkanSwapchain();
	DrawImage = new PVulkanImage();
	DepthImage = new PVulkanImage();
	RenderGraph = new PVulkanRenderGraph();
	OverlayRenderGraph = new PVulkanRenderGraph();
	ParallelFramePool = new PVulkanFramePool(DeferredFrameCount);
	ImmediateFramePool = new PVulkanFramePool(ImmediateFrameCount);
	GOverlay = new PVulkanOverlay();

	Allocator->Init();
	Swapchain->Init();
	
	DrawImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_R16G16B16A16_SFLOAT);
	DrawImage->CreateImage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	DrawImage->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
	
	DepthImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_D32_SFLOAT);
	DepthImage->CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	DepthImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	
	ParallelFramePool->CreateFramePool();
	ImmediateFramePool->CreateFramePool(); // TODO: Should not do full initialization on ImmediateFramePool.

	GOverlay->Init();

	static const FString a = "DS_RESOURCE_SSBO";
	static const FString b = "DS_RESOURCE_SSIO";
	static const FString c = "DS_RESOURCE_TEX2DS";

	for (PVulkanFrame* Frame : *ParallelFramePool)
	{
		FVkDescriptorSet DescriptorSetSSBO;

		Frame->DescriptorSets.Insert("DSSSBO", DescriptorSetSSBO);
		Frame->DescriptorSets.Insert("DSSSIO", DescriptorSetSSBO);
	}
















	// TODO: Temporary testing

	MeshBuffer = MakeShared<FVkMeshBuffer>();
	MeshBuffer->Initialize();

	SMeshBinaryData MeshData;
    PGLTF::ImportGLTF("/home/user/Workspace/Game/Game/Content/Cube.glb", MeshData);
	for (int i = 0; i < 1000; ++i)
	{
		MeshBuffer->AddData(MeshData.Vertices, MeshData.Indices);
	}

	RenderGraph->AddCommand([&](PVulkanFrame* Frame) mutable
	{
		struct FCameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
		} CameraData;

		CameraData.Projection = GetScene()->GetCamera()->GetProjectionMatrix();
		CameraData.View = GetScene()->GetCamera()->GetViewMatrix();
		
		Frame->GlobalStorageBuffer->Submit(&CameraData, sizeof(FCameraData), 0);
	});

	RenderGraph->AddCommand([&](PVulkanFrame* Frame)
	{
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindPipeline(Frame->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, ParallelFramePool->GraphicsPipeline->Info.Handle);
		Frame->StorageBufferDescriptorSet->Bind(ParallelFramePool->GraphicsPipelineLayout);
		vkCmdBindVertexBuffers(Frame->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &MeshBuffer->VertexBuffer->Info.Handle, offsets);
		vkCmdBindIndexBuffer(Frame->GetCommandBuffer()->GetVkCommandBuffer(), MeshBuffer->IndexBuffer->Info.Handle, 0, VK_INDEX_TYPE_UINT32);
		MeshBuffer->DrawIndirect(Frame->GetCommandBuffer());
	});
}

void PVulkanSceneRenderer::Shutdown()
{
	GOverlay->Shutdown();
	ParallelFramePool->FreeFramePool();
	ImmediateFramePool->FreeFramePool();
	DrawImage->DestroyImage();
	DrawImage->DestroyImageView();
	DepthImage->DestroyImage();
	DepthImage->DestroyImageView();
	Swapchain->Shutdown();
	Allocator->Shutdown();

	delete GOverlay;
	delete ParallelFramePool;
	delete ImmediateFramePool;
	delete DrawImage;
	delete DepthImage;
	delete Swapchain;
	delete Allocator;
	
	GOverlay = nullptr;
}

void PVulkanSceneRenderer::Resize()
{
	Swapchain->Shutdown();
	Swapchain->Init();

	DrawImage->DestroyImage();
	DrawImage->DestroyImageView();
	DrawImage->Reset();
	DrawImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_R16G16B16A16_SFLOAT);
	DrawImage->CreateImage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	DrawImage->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	DepthImage->DestroyImage();
	DepthImage->DestroyImageView();
	DepthImage->Reset();
	DepthImage->Init(GetSwapchain()->GetVkExtent(), VK_FORMAT_D32_SFLOAT);
	DepthImage->CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	DepthImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);	
}

void PVulkanSceneRenderer::Render()
{
	PROFILE_FUNC_SCOPE("PVulkanSceneRenderer::Render")
	
	PVulkanFrame* Frame = ParallelFramePool->GetCurrentFrame();
	Frame->BeginFrame();

	DrawImage->TransitionImageLayout(Frame->GetCommandBuffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	DepthImage->TransitionImageLayout(Frame->GetCommandBuffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	RenderGraph->BeginRendering();
	RenderGraph->Execute(Frame);
	RenderGraph->EndRendering();
	DrawImage->TransitionImageLayout(Frame->GetCommandBuffer(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	Swapchain->GetSwapchainImages()[Frame->GetTransientFrameData().NextImageIndex]->TransitionImageLayout(Frame->GetCommandBuffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	DrawImage->CopyImageRegion(Frame->GetCommandBuffer(), Swapchain->GetSwapchainImages()[Frame->GetTransientFrameData().NextImageIndex]->GetVkImage(), DrawImage->GetImageExtent2D(), Swapchain->GetVkExtent());
	OverlayRenderGraph->Execute(Frame);
	Swapchain->GetSwapchainImages()[Frame->GetTransientFrameData().NextImageIndex]->TransitionImageLayout(Frame->GetCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	Frame->EndFrame();
	ParallelFramePool->FrameIndex++;
}

PVulkanAllocator* PVulkanSceneRenderer::GetAllocator() const
{
	return Allocator;
}

PVulkanSwapchain* PVulkanSceneRenderer::GetSwapchain() const
{
	return Swapchain;
}

PVulkanImage* PVulkanSceneRenderer::GetDrawImage() const
{
	return DrawImage;
}

PVulkanImage* PVulkanSceneRenderer::GetDepthImage() const
{
    return DepthImage;
}

PVulkanRenderGraph* PVulkanSceneRenderer::GetRenderGraph() const
{
	return RenderGraph;
}

PVulkanRenderGraph* PVulkanSceneRenderer::GetOverlayRenderGraph() const
{
	return OverlayRenderGraph;
}

PVulkanFramePool* PVulkanSceneRenderer::GetParallelFramePool() const
{
	return ParallelFramePool;
}

// TODO: Move to Command
void PVulkanSceneRenderer::ImmediateSubmit(std::function<void(PVulkanCommandBuffer* CommandBuffer)>&& Func)
{
	VkCommandBuffer CommandBufferPointer = ImmediateFramePool->Pool[0]->GetCommandBuffer()->GetVkCommandBuffer();

	VkFence RenderFence = ImmediateFramePool->Pool[0]->GetRenderFence();

	VkResult Result = vkResetFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset fence.");

	Result = vkResetCommandBuffer(CommandBufferPointer, 0);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");
	
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.pNext = nullptr;
	CommandBufferBeginInfo.pInheritanceInfo = nullptr;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	Result = vkBeginCommandBuffer(CommandBufferPointer, &CommandBufferBeginInfo);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to begin command buffer.");

	Func(ImmediateFramePool->Pool[0]->GetCommandBuffer());

	Result = vkEndCommandBuffer(CommandBufferPointer);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to end command buffer..");

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo{};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.pNext = nullptr;
	CommandBufferSubmitInfo.commandBuffer = CommandBufferPointer;
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
	Result = vkQueueSubmit2(GetRHI()->GetDevice()->GetGraphicsQueue(), 1, &SubmitInfo, RenderFence);
	Result = vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence, true, UINT64_MAX);
}