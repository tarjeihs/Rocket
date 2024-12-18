#include "EnginePCH.h"
#include "VulkanRenderer.h"

#include "Renderer/Common/Shader.h"
#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Types/UniquePtr.h"
#include "Utils/Profiler.h"
#include "VulkanDescriptor.h"

void FVulkanRenderer::Init()
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
}

void FVulkanRenderer::Shutdown()
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
}

void FVulkanRenderer::Render()
{
	PROFILE_FUNC_SCOPE("FVulkanRenderer::Render")

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

void FVulkanRenderer::Resize()
{
	PROFILE_FUNC_SCOPE("FVulkanRenderer::Resize")

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

void FVulkanRenderer::BeginFrame()
{
    PROFILE_FUNC_SCOPE("FVulkanRenderer::BeginFrame")

	vkWaitForFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence[FrameIndex], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(GetRHI()->GetDevice()->GetVkDevice(), Swapchain->GetVkSwapchain(), UINT64_MAX, SwapchainSemaphore[FrameIndex], VK_NULL_HANDLE, &NextImageIndex[FrameIndex]);
	vkResetFences(GetRHI()->GetDevice()->GetVkDevice(), 1, &RenderFence[FrameIndex]);
	
	CommandBuffer[FrameIndex]->ResetCommandBuffer();
	CommandBuffer[FrameIndex]->BeginCommandBuffer();
}

void FVulkanRenderer::EndFrame()
{
	PROFILE_FUNC_SCOPE("FVulkanRenderer::EndFrame")

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

void FVulkanRenderer::ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func)
{
	PROFILE_FUNC_SCOPE("FVulkanRenderer::ImmediateSubmit")

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

void FVulkanForwardRenderer::Init()
{
    Super::Init();

	FScriptableRenderPipeline* OpaqueRenderPipeline = new FOpaqueRenderPipeline();
	OpaqueRenderPipeline->Initialize();
	ScriptableRenderPipeline.Insert("Opaque", OpaqueRenderPipeline);
}

void FVulkanForwardRenderer::Shutdown() 
{
    Super::Shutdown();

	FScriptableRenderPipeline** OpaqueRenderPipeline = ScriptableRenderPipeline.Find("Opaque");
	if (OpaqueRenderPipeline)
	{
		FScriptableRenderPipeline* Found = *OpaqueRenderPipeline;
		Found->Shutdown();
	}
}

void FVulkanForwardRenderer::Bind()
{
	FScriptableRenderPipeline** OpaqueRenderPipeline = ScriptableRenderPipeline.Find("Opaque");
	if (OpaqueRenderPipeline)
	{
		FScriptableRenderPipeline* Found = *OpaqueRenderPipeline;
		Found->Bind();
	}
}

FVkMeshBuffer* MeshBuffer = new FVkMeshBuffer();

void FOpaqueRenderPipeline::Initialize()
{
	FShaderCreateInfo VertexShaderCreateInfo;
	VertexShaderCreateInfo.Stage = EShaderStage::Vertex;
	VertexShaderCreateInfo.Path = RK_ENGINE_DIR "/Shaders/HLSL/OpaqueVS.hlsl";

	FShaderCreateInfo PixelShaderCreateInfo;
	PixelShaderCreateInfo.Stage = EShaderStage::Fragment;
	PixelShaderCreateInfo.Path = RK_ENGINE_DIR "/Shaders/HLSL/OpaquePS.hlsl";

	VertexShader = MakeUnique<FVkShader>();
	PixelShader = MakeUnique<FVkShader>();

	VertexShader->CreateShader(VertexShaderCreateInfo);
	PixelShader->CreateShader(PixelShaderCreateInfo);

	Pipeline = MakeUnique<FVkPipeline>();
	PipelineLayout = MakeUnique<FVkPipelineLayout>();

	MeshBuffer->Initialize();

	TArray<FVkDescriptorLayout> StorageBufferDescriptorLayout = {
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 }
	};
	FVkDescriptorSetLayoutCreateInfo StorageBufferDescriptorSetLayoutCreateInfo = { .Descriptors = StorageBufferDescriptorLayout };
	FVkDescriptorSetLayout* StorageBufferDescriptorSetLayout = new FVkDescriptorSetLayout();
	StorageBufferDescriptorSetLayout->Initialize(StorageBufferDescriptorSetLayoutCreateInfo);
	
	FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo;
	PipelineLayoutCreateInfo.DescriptorSetLayouts = {
		StorageBufferDescriptorSetLayout->Info.Handle
	};
	PipelineLayout->Initialize(PipelineLayoutCreateInfo);
	PipelineLayout->Info.DescriptorSetLayout.Add(StorageBufferDescriptorSetLayout);

	FVkPipelineCreateInfo PipelineCreateInfo;
	PipelineCreateInfo.PipelineLayout = PipelineLayout.Get();
	PipelineCreateInfo.Shaders = { PixelShader.Get(), VertexShader.Get() };
	Pipeline->Initialize(PipelineCreateInfo);
	for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
	{
		TArray<FVkDescriptorPoolRatio> PoolRatio = { { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 }, };
		FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
		DescriptorPoolCreateInfo.PoolRatios = PoolRatio;
		DescriptorPoolCreateInfo.MaxSetCount = 1;
		DescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		FVkDescriptorPool* DescriptorPool = new FVkDescriptorPool();
		DescriptorPool->Initialize(DescriptorPoolCreateInfo);
		
		FVkBufferCreateInfo StorageBufferCreateInfo;
		StorageBufferCreateInfo.Size = 64 * 1024 * 1024;
		StorageBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		StorageBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_CPU_TO_GPU;

		FVkBuffer* GlobalStorageBuffer = new FVkBuffer();
		FVkBuffer* CameraStorageBuffer = new FVkBuffer();
		FVkBuffer* MaterialStorageBuffer = new FVkBuffer();
		FVkBuffer* ObjectStorageBuffer = new FVkBuffer();
		
		GlobalStorageBuffer->Initialize(StorageBufferCreateInfo);
		CameraStorageBuffer->Initialize(StorageBufferCreateInfo);
		MaterialStorageBuffer->Initialize(StorageBufferCreateInfo);
		ObjectStorageBuffer->Initialize(StorageBufferCreateInfo);

		FVkDescriptorSetCreateInfo StorageBufferDescriptorSetCreateInfo { .DescriptorPool = DescriptorPool, .DescriptorSetLayout = StorageBufferDescriptorSetLayout };
		FVkDescriptorSet* StorageBufferDescriptorSet = new FVkDescriptorSet();
		StorageBufferDescriptorSet->Initialize(StorageBufferDescriptorSetCreateInfo);
		StorageBufferDescriptorSet->WriteBuffer(0, GlobalStorageBuffer);
		StorageBufferDescriptorSet->WriteBuffer(1, CameraStorageBuffer);
		StorageBufferDescriptorSet->WriteBuffer(2, MaterialStorageBuffer);
		StorageBufferDescriptorSet->WriteBuffer(3, ObjectStorageBuffer);
		StorageBufferDescriptorSet->Info.DescriptorPool = DescriptorPool;
		Pipeline->Info.DescriptorSet[Index].Add(StorageBufferDescriptorSet);
	}
}

void FOpaqueRenderPipeline::Shutdown()
{
	PixelShader->Shutdown();
	VertexShader->Shutdown();
	PipelineLayout->Shutdown();
	Pipeline->Shutdown();
	MeshBuffer->Shutdown();
}

void FOpaqueRenderPipeline::Bind()
{
	PROFILE_FUNC_SCOPE("FVulkanRenderer::Bind")

	const PVulkanCommandBuffer* CommandBuffer = GetRHI()->GetRenderer()->GetCommandBuffer();
	const SizeType FrameIndex = GetRHI()->GetRenderer()->GetFrameIndex();
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(CommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Info.Handle);
	for (SizeType Index = 0; Index < Pipeline->Info.DescriptorSet[FrameIndex].GetSize(); ++Index)
	{
		Pipeline->Info.DescriptorSet[FrameIndex][Index]->Bind(PipelineLayout.Get());
	}
	vkCmdBindVertexBuffers(CommandBuffer->GetVkCommandBuffer(), 0, 1, &MeshBuffer->VertexBuffer->Info.Handle, offsets);
	vkCmdBindIndexBuffer(CommandBuffer->GetVkCommandBuffer(), MeshBuffer->IndexBuffer->Info.Handle, 0, VK_INDEX_TYPE_UINT32);
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