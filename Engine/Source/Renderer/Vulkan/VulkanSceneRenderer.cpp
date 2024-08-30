#include "VulkanSceneRenderer.h"

#include "Core/Assert.h"
#include "Core/Camera.h"
#include "Scene/Scene.h"
#include "Math/Transform.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanImGui.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanMemory.h"

// 3 swapchain images, 2 frames.
// 1 frame is currently being processed by CPU, 1 is being drawn by GPU, hence no need for a third frame.
static constexpr size_t DeferredFrameCount = 2;
static constexpr size_t ImmediateFrameCount = 1;

//> init_data
std::vector<SVertex> cube_vertices()
{
	std::vector<SVertex> cube_vertices(8);
	cube_vertices[0].position = { 0.5f,  0.5f,  0.5f }; // Front-top-right
	cube_vertices[1].position = { 0.5f, -0.5f,  0.5f }; // Front-bottom-right
	cube_vertices[2].position = { -0.5f, -0.5f,  0.5f }; // Front-bottom-left
	cube_vertices[3].position = { -0.5f,  0.5f,  0.5f }; // Front-top-left
	cube_vertices[4].position = { 0.5f,  0.5f, -0.5f }; // Back-top-right
	cube_vertices[5].position = { 0.5f, -0.5f, -0.5f }; // Back-bottom-right
	cube_vertices[6].position = { -0.5f, -0.5f, -0.5f }; // Back-bottom-left
	cube_vertices[7].position = { -0.5f,  0.5f, -0.5f }; // Back-top-left

	cube_vertices[0].color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red
	cube_vertices[1].color = { 0.0f, 1.0f, 0.0f, 1.0f }; // Green
	cube_vertices[2].color = { 0.0f, 0.0f, 1.0f, 1.0f }; // Blue
	cube_vertices[3].color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
	cube_vertices[4].color = { 1.0f, 0.0f, 1.0f, 1.0f }; // Magenta
	cube_vertices[5].color = { 0.0f, 1.0f, 1.0f, 1.0f }; // Cyan
	cube_vertices[6].color = { 1.0f, 1.0f, 1.0f, 1.0f }; // White
	cube_vertices[7].color = { 0.5f, 0.5f, 0.5f, 1.0f }; // Grey

	return cube_vertices;
}
std::vector<uint32_t> cube_indices = {
    // Front face
    0, 3, 2, 2, 1, 0,
    // Right face
    0, 1, 5, 5, 4, 0,
    // Back face
    4, 5, 6, 6, 7, 4,
    // Left face
    7, 6, 2, 2, 3, 7,
    // Top face
    7, 3, 0, 0, 4, 7,
    // Bottom face
    1, 2, 6, 6, 5, 1
};

PVulkanMesh* Mesh = nullptr;

void PVulkanSceneRenderer::Init()
{
	Swapchain = new PVulkanSwapchain();
	RenderTarget = new PVulkanImage();
	RenderGraph = new PVulkanRenderGraph();
	DeferredFramePool = new PVulkanFramePool(DeferredFrameCount);
	ImmediateFramePool = new PVulkanFramePool(ImmediateFrameCount);
	ImGui = new PVulkanImGui();
	Mesh = new PVulkanMesh();
	GraphicsPipeline = new PVulkanGraphicsPipeline();

	Swapchain->Init(RHI);
	RenderTarget->CreateImage(RHI, Swapchain->GetVkExtent());
	DeferredFramePool->CreateFramePool(RHI);
	ImmediateFramePool->CreateFramePool(RHI);
	ImGui->Init(RHI);
	Mesh->CreateMesh(RHI, cube_vertices(), cube_indices);
	GraphicsPipeline->CreatePipeline(RHI);

	RenderGraph->AddCommand([this](VkCommandBuffer CommandBuffer)
	{
		PVulkanFrame* Frame = DeferredFramePool->Pool[DeferredFramePool->FrameIndex % DeferredFrameCount];

		static STransform Transform;
		Transform.Rotation.x += 0.0001f;
		Transform.Rotation.y += 0.0003f;

		SShaderStorageBufferObject SSBO;
		PCamera* camera = GetScene()->GetCamera();
		SSBO.ModelMatrix = Transform.ToMatrix();
		RHI->GetMemory()->UploadBuffer(GraphicsPipeline->StorageBuffer, &SSBO, sizeof(SShaderStorageBufferObject));

		// This does not work atm because of us not actually passing 1024 * 10 elements, but one single element..
		//RHI->GetMemory()->UploadBuffer(GraphicsPipeline->StorageBuffer, &SSBO, sizeof(SShaderStorageBufferObject) * 1024 * 10);



		static STransform transform;
		transform.Rotation.x += 0.0001f;
		transform.Rotation.y += 0.0003f;

		SUniformBufferObject uniformBuffer;
		uniformBuffer.ViewMatrix = camera->GetViewMatrix();
		uniformBuffer.ProjectionMatrix = camera->m_Projection;
		uniformBuffer.WorldMatrix = transform.ToMatrix();
		GraphicsPipeline->BindPipeline(RHI, Frame->GetCommandBuffer(), Mesh, uniformBuffer);
	});
}

void PVulkanSceneRenderer::Shutdown()
{
	GraphicsPipeline->DestroyPipeline(RHI);
	Mesh->DestroyMesh(RHI);
	ImGui->Shutdown(RHI);
	DeferredFramePool->FreeFramePool(RHI);
	ImmediateFramePool->FreeFramePool(RHI);
	RenderTarget->DestroyImage(RHI);
	Swapchain->Shutdown(RHI);

	delete Mesh;
	delete GraphicsPipeline;
	delete ImGui;
	delete DeferredFramePool;
	delete ImmediateFramePool;
	delete RenderTarget;
	delete Swapchain;
}

void PVulkanSceneRenderer::Resize()
{
	Swapchain->Shutdown(RHI);
	Swapchain->Init(RHI);

	RenderTarget->DestroyImage(RHI);
	RenderTarget->CreateImage(RHI, Swapchain->GetVkExtent());
}

void PVulkanSceneRenderer::Render()
{
	PVulkanFrame* Frame = DeferredFramePool->Pool[DeferredFramePool->FrameIndex % DeferredFrameCount];
	Frame->BeginFrame(RHI);

	TransitionImageLayout(Frame->GetCommandBuffer()->GetVkCommandBuffer(), RenderTarget->Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	RenderGraph->Execute(Frame->GetCommandBuffer()->GetVkCommandBuffer());
	TransitionImageLayout(Frame->GetCommandBuffer()->GetVkCommandBuffer(), RenderTarget->Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	TransitionImageLayout(Frame->GetCommandBuffer()->GetVkCommandBuffer(), Swapchain->GetSwapchainImages()[Frame->TransientFrameData.NextImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyImageRegion(Frame->GetCommandBuffer()->GetVkCommandBuffer(), RenderTarget->Image, Swapchain->GetSwapchainImages()[Frame->TransientFrameData.NextImageIndex], RenderTarget->ImageExtent, Swapchain->GetVkExtent());
	TransitionImageLayout(Frame->GetCommandBuffer()->GetVkCommandBuffer(), Swapchain->GetSwapchainImages()[Frame->TransientFrameData.NextImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	ImGui->Bind(RHI, Frame->GetCommandBuffer(), Swapchain->GetSwapchainImageViews()[Frame->TransientFrameData.NextImageIndex]);

	Frame->EndFrame(RHI);
	DeferredFramePool->FrameIndex++;
}

PVulkanSwapchain* PVulkanSceneRenderer::GetSwapchain() const
{
	return Swapchain;
}

PVulkanImage* PVulkanSceneRenderer::GetRenderTarget() const
{
	return RenderTarget;
}

PVulkanRenderGraph * PVulkanSceneRenderer::GetRenderGraph() const
{
	return RenderGraph;
}

void PVulkanSceneRenderer::ImmediateSubmit(std::function<void(PVulkanCommandBuffer* CommandBuffer)>&& Func)
{
	VkCommandBuffer CommandBufferPointer = ImmediateFramePool->Pool[0]->CommandBuffer->GetVkCommandBuffer();

	VkResult Result = vkResetFences(RHI->GetDevice()->GetVkDevice(), 1, &ImmediateFramePool->Pool[0]->RenderFence);
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

	Func(ImmediateFramePool->Pool[0]->CommandBuffer);

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
	Result = vkQueueSubmit2(RHI->GetDevice()->GetGraphicsQueue(), 1, &SubmitInfo, ImmediateFramePool->Pool[0]->RenderFence);
	Result = vkWaitForFences(RHI->GetDevice()->GetVkDevice(), 1, &ImmediateFramePool->Pool[0]->RenderFence, true, UINT64_MAX);
}

void PVulkanSceneRenderer::TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout CurrentLayout, VkImageLayout NewLayout)
{
	VkImageSubresourceRange SubresourceRange{};
	SubresourceRange.aspectMask = (NewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	SubresourceRange.baseMipLevel = 0;
	SubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	SubresourceRange.baseArrayLayer = 0;
	SubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	VkImageMemoryBarrier2 ImageMemoryBarrier{};
	ImageMemoryBarrier.pNext = nullptr;
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	ImageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageMemoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	ImageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	ImageMemoryBarrier.oldLayout = CurrentLayout;
	ImageMemoryBarrier.newLayout = NewLayout;
	ImageMemoryBarrier.subresourceRange = SubresourceRange;
	ImageMemoryBarrier.image = Image;

	VkDependencyInfo DependencyInfo{};
	DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	DependencyInfo.pNext = nullptr;
	DependencyInfo.imageMemoryBarrierCount = 1;
	DependencyInfo.pImageMemoryBarriers = &ImageMemoryBarrier;

	vkCmdPipelineBarrier2(CommandBuffer, &DependencyInfo);
}

void PVulkanSceneRenderer::CopyImageRegion(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize)
{
	VkImageBlit2 ImageBlit{};
	ImageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	ImageBlit.pNext = nullptr;
	ImageBlit.srcOffsets[1].x = SrcSize.width;
	ImageBlit.srcOffsets[1].y = SrcSize.height;
	ImageBlit.srcOffsets[1].z = 1;
	ImageBlit.dstOffsets[1].x = DstSize.width;
	ImageBlit.dstOffsets[1].y = DstSize.height;
	ImageBlit.dstOffsets[1].z = 1;
	ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageBlit.srcSubresource.baseArrayLayer = 0;
	ImageBlit.srcSubresource.layerCount = 1;
	ImageBlit.srcSubresource.mipLevel = 0;
	ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageBlit.dstSubresource.baseArrayLayer = 0;
	ImageBlit.dstSubresource.layerCount = 1;
	ImageBlit.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 ImageBlitInfo{};
	ImageBlitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	ImageBlitInfo.pNext = nullptr;
	ImageBlitInfo.dstImage = Dest;
	ImageBlitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ImageBlitInfo.srcImage = Src;
	ImageBlitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	ImageBlitInfo.filter = VK_FILTER_LINEAR;
	ImageBlitInfo.regionCount = 1;
	ImageBlitInfo.pRegions = &ImageBlit;

	vkCmdBlitImage2(CommandBuffer, &ImageBlitInfo);
}