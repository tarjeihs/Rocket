// Test Mesh during culling (compute)
// Select correct LOD level, and generate a draw indexed indirect command for that LOD. Other LODs are ignored.

#include "EnginePCH.h"
#include "VkCullingComputePipeline.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

void FVkCullingComputePipeline::Initialize(FVkPipelineLayout* PipelineLayout)
{

}

void FVkCullingComputePipeline::Shutdown()
{

}

void FVkCullingComputePipeline::Execute()
{
    vkCmdDispatch(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 64, 64, 1);

    // Barrier?

    vkCmdDrawIndexedIndirect(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), DrawCommandBuffer->Info.Handle, 0, 100, 20);
}