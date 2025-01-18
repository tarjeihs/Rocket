#include "EnginePCH.h"
#include "VkOpaqueSkinnedGfxPipeline.h"
#include "Renderer/Allocators/VkMeshAllocator.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"

void FVkOpaqueSkinnedMeshGfxPipeline::Initialize(FVkPipelineLayout* PipelineLayout)
{
    //TUniquePtr<FVkShader> VertexShader = MakeUnique<FVkShader>();
	//TUniquePtr<FVkShader> PixelShader = MakeUnique<FVkShader>();
//
	//FShaderCreateInfo VertexShaderCreateInfo
	//{
	//	RK_ENGINE_DIR "/Shaders/HLSL/OpaqueSkinnedMeshVS.hlsl", EShaderStage::Vertex
	//};
//
	//FShaderCreateInfo PixelShaderCreateInfo
	//{
	//	RK_ENGINE_DIR "/Shaders/HLSL/OpaqueSkinnedMeshPS.hlsl", EShaderStage::Pixel
	//};
//
	//TArray<FVkVertexAttribute> Attributes
	//{
	//	{
	//		0,
	//		0,
	//		VK_FORMAT_R32G32B32_SFLOAT,
	//		offsetof(FSkeletalVertex, Position),
	//		sizeof(FSkeletalVertex)
	//	},
	//	{
	//		0,
	//		1,
	//		VK_FORMAT_R32G32B32_SFLOAT,
	//		offsetof(FSkeletalVertex, Normal),
	//		sizeof(FSkeletalVertex)
	//	},
	//	{
	//		0,
	//		2,
	//		VK_FORMAT_R32G32_SFLOAT,
	//		offsetof(FSkeletalVertex, TexCoord),
	//		sizeof(FSkeletalVertex)
	//	},
	//	{
	//		0,
	//		3,
	//		VK_FORMAT_R32G32B32A32_SFLOAT,
	//		offsetof(FSkeletalVertex, Weight),
	//		sizeof(FSkeletalVertex)
	//	},
	//	{
	//		0,
	//		4,
	//		VK_FORMAT_R32G32B32A32_SINT,
	//		offsetof(FSkeletalVertex, Indices),
	//		sizeof(FSkeletalVertex)
	//	}
	//};
//
    //FVkPipelineCreateInfo PipelineCreateInfo
	//{
	//	{
	//		VertexShader.Get(), 
	//		PixelShader.Get()
	//	},
	//	Attributes,
	//	PipelineLayout
	//};
//
    //VertexShader->Init(VertexShaderCreateInfo);
	//PixelShader->Init(PixelShaderCreateInfo);
//
	//Pipeline = MakeUnique<FVkPipeline>();
	//Pipeline->Initialize(PipelineCreateInfo);
//
    //VertexShader->Free();
    //PixelShader->Free();
}

void FVkOpaqueSkinnedMeshGfxPipeline::Shutdown()
{
   //Pipeline->Shutdown();
}

void FVkOpaqueSkinnedMeshGfxPipeline::Execute()
{
	//PROFILE_FUNC_SCOPE("FVkOpaqueStaticMeshGfxPipeline::Execute")
//
    //PVulkanCommandBuffer* CommandBuffer = GetRHI()->GetRenderer()->GetCommandBuffer();
	//VkDeviceSize Offsets[] = { 0 };
//
    //VkRenderingAttachmentInfo ColorAttachment = {};
    //ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    //ColorAttachment.pNext = nullptr;
    //ColorAttachment.imageView = GetRHI()->GetRenderer()->GetColorAttachment16()->Info.ImageViewHandle;
    //ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    //ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //ColorAttachment.clearValue = { 0.0033f, 0.0033f, 0.0033f, 1.0f };
//
    //VkRenderingAttachmentInfo DepthAttachment{};
    //DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    //DepthAttachment.pNext = nullptr;
    //DepthAttachment.imageView = GetRHI()->GetRenderer()->GetDepthAttachmentD32()->Info.ImageViewHandle;
    //DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    //DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    //DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //DepthAttachment.clearValue.depthStencil.depth = 1.0f;
//
    //VkRenderingInfo RenderingInfo{};
    //RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    //RenderingInfo.pNext = nullptr;
    //RenderingInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, VkExtent2D { GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width, GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height }};
    //RenderingInfo.layerCount = 1;
    //RenderingInfo.colorAttachmentCount = 1;
    //RenderingInfo.pColorAttachments = &ColorAttachment;
    //RenderingInfo.pDepthAttachment = &DepthAttachment;
    //RenderingInfo.pStencilAttachment = nullptr;
//
    //VkViewport Viewport{};
    //Viewport.x = 0;
    //Viewport.y = 0;
    //Viewport.width = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width;
    //Viewport.height = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height;
    //Viewport.minDepth = 0.0f;
    //Viewport.maxDepth = 1.0f;
//
    //VkRect2D Scissor = {};
    //Scissor.offset.x = 0;
    //Scissor.offset.y = 0;
    //Scissor.extent.width = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width;
    //Scissor.extent.height = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height;
//
    //vkCmdBeginRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
    //vkCmdSetViewport(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Viewport);
    //vkCmdSetScissor(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Scissor);
    //
    //vkCmdBindPipeline(CommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Info.Handle);
	////vkCmdBindVertexBuffers(CommandBuffer->GetVkCommandBuffer(), 0, 1, &SkeletalMeshBuffer->Info.VertexBuffer->Info.Handle, Offsets);
	////vkCmdBindIndexBuffer(CommandBuffer->GetVkCommandBuffer(), SkeletalMeshBuffer->Info.IndexBuffer->Info.Handle, 0, VK_INDEX_TYPE_UINT32);
	//
	////SkeletalMeshBuffer->DrawIndexedIndirect(CommandBuffer);
//
    //vkCmdEndRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer());   
}