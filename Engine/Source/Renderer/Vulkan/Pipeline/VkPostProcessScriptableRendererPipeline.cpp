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
#include <vulkan/vulkan_core.h>

void FVkPostProcessScriptableRendererPipeline::Initialize(FVkPipelineLayout* PipelineLayout)
{
    TUniquePtr<FVkShader> ComputeShader  = MakeUnique<FVkShader>();

    FShaderCreateInfo ComputeShaderCreateInfo
	{
		RK_ENGINE_DIR "/Shaders/HLSL/Tone.hlsl", EShaderStage::Compute
	};

    FVkPipelineCreateInfo PipelineCreateInfo
	{
		{
			ComputeShader.Get()
		},
		PipelineLayout
	};

    ComputeShader->Init(ComputeShaderCreateInfo);
    
    Pipeline = MakeUnique<FVkPipeline>();
	Pipeline->InitCompute(PipelineCreateInfo);

    ComputeShader->Free();
}

void FVkPostProcessScriptableRendererPipeline::Shutdown()
{
    Pipeline->Shutdown();
}

void FVkPostProcessScriptableRendererPipeline::Execute()
{
    PROFILE_FUNC_SCOPE("FVkPostProcessScriptableRendererPipeline::Execute")
    
    PVulkanCommandBuffer* CommandBuffer = GetRHI()->GetRenderer()->GetCommandBuffer();

    FVkImage* DrawImage = GetRHI()->GetRenderer()->ColorAttachment16.Get();
    FVkImage* FinalImage = GetRHI()->GetRenderer()->ColorAttachment8.Get();

    FVkImage* HDR = (*GetRHI()->GetRenderer()->RWTexture2D.Find("HDR"))[GetRHI()->GetRenderer()->GetFrameIndex()];
    FVkImage* SDR = (*GetRHI()->GetRenderer()->RWTexture2D.Find("SDR"))[GetRHI()->GetRenderer()->GetFrameIndex()];

    HDR->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    SDR->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    DrawImage->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    DrawImage->CopyImageRegion(CommandBuffer, HDR->Info.ImageHandle, DrawImage->Info.Extent, HDR->Info.Extent);
    DrawImage->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    vkCmdBindPipeline(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline->Info.Handle);
    vkCmdDispatch(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), std::ceil(HDR->Info.Extent.width / 16), std::ceil(HDR->Info.Extent.height / 16), 1);

    SDR->TransitionImageLayout(CommandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    SDR->CopyImageRegion(CommandBuffer, FinalImage->Info.ImageHandle, SDR->Info.Extent, FinalImage->Info.Extent);
}