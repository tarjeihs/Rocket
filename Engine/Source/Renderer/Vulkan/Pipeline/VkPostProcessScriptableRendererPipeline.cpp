#include "EnginePCH.h"
#include "VkPostProcessScriptableRendererPipeline.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Types/Vertex.h"

void FVkPostProcessComputePipeline::Initialize(FVkPipelineLayout* PipelineLayout)
{
    TUniquePtr<FVkShader> ComputeShader  = MakeUnique<FVkShader>();

    FShaderCreateInfo ComputeShaderCreateInfo
	{
		RK_ENGINE_DIR "/Shaders/HLSL/PostProcess.hlsl", EShaderStage::Compute
	};

	TArray<FVkVertexAttribute> Attributes
	{
	};

    FVkPipelineCreateInfo PipelineCreateInfo
	{
		{
			ComputeShader.Get()
		},
        Attributes,
		PipelineLayout
	};

    ComputeShader->Init(ComputeShaderCreateInfo);
    
    Pipeline = MakeUnique<FVkPipelineCompute>();
	Pipeline->Initialize(PipelineCreateInfo);

    ComputeShader->Free();
}

void FVkPostProcessComputePipeline::Shutdown()
{
    Pipeline->Shutdown();
}

void FVkPostProcessComputePipeline::Execute()
{
    PROFILE_FUNC_SCOPE("FVkPostProcessComputePipeline::Execute")
    
    FVkCommandBuffer* CommandBuffer = GetRHI()->GetRenderer()->GetCommandBuffer();

    FVkImage* IntermediateColorAttachment = GetRHI()->GetRenderer()->IntermediateColorAttachment.Get();
    FVkImage* PresentColorAttachment = GetRHI()->GetRenderer()->PresentColorAttachment.Get();

    FVkImage* TonemapInputImage = (*GetRHI()->GetRenderer()->RWTexture2D.Find("HDR"))[GetRHI()->GetRenderer()->GetFrameIndex()];
    FVkImage* TonemapOutputImage = (*GetRHI()->GetRenderer()->RWTexture2D.Find("SDR"))[GetRHI()->GetRenderer()->GetFrameIndex()];

    TonemapInputImage->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    TonemapOutputImage->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    
    IntermediateColorAttachment->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    IntermediateColorAttachment->CopyImageRegion(CommandBuffer, TonemapInputImage->Info.ImageHandle, IntermediateColorAttachment->Info.Extent, TonemapInputImage->Info.Extent);
    IntermediateColorAttachment->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    
    const uint32 GroupCountX = (GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width + 15) / 16;
    const uint32 GroupCountY = (GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height + 15) / 16;
    
    vkCmdBindPipeline(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline->Info.Handle);
    vkCmdDispatch(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), GroupCountX, GroupCountY, 1);
    
    TonemapOutputImage->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TonemapOutputImage->CopyImageRegion(CommandBuffer, PresentColorAttachment->Info.ImageHandle, TonemapOutputImage->Info.Extent, PresentColorAttachment->Info.Extent);
}