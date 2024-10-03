#include "EnginePCH.h"
#include "VulkanMesh.h"

#include "Renderer/Common/Material.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Utils/Profiler.h"

void PVulkanMesh::CreateMesh(const SMeshBinaryData& MeshBinaryObject)
{
    const size_t VertexBufferSize = MeshBinaryObject.Vertices.size() * sizeof(SVertex);
    const size_t IndexBufferSize = MeshBinaryObject.Indices.size() * sizeof(uint32_t);

    VertexBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    IndexBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    StagingBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    VertexBuffer->Allocate(VertexBufferSize);
    IndexBuffer->Allocate(IndexBufferSize);
    StagingBuffer->Allocate(VertexBufferSize + IndexBufferSize);

    void* Data = nullptr;
    vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Allocation, &Data);
    memcpy(Data, MeshBinaryObject.Vertices.data(), VertexBufferSize);
    memcpy((char*)Data + VertexBufferSize, MeshBinaryObject.Indices.data(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Allocation);

    VkBufferDeviceAddressInfo BufferDeviceAddressInfo{};
    BufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    BufferDeviceAddressInfo.buffer = VertexBuffer->Buffer;
    DeviceAddress64 = vkGetBufferDeviceAddress(GetRHI()->GetDevice()->GetVkDevice(), &BufferDeviceAddressInfo);

    // Copy vertex and index data from the staging buffer (in CPU memory) immediately but synchronously to the GPU-only buffers, ensuring the data is guaranteed to be in GPU memory and ready for use.
    GetRHI()->GetSceneRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        VkBufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = VertexBufferSize;

        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Buffer, VertexBuffer->Buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{ 0 };
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = VertexBufferSize;
        indexCopy.size = IndexBufferSize;

        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Buffer, IndexBuffer->Buffer, 1, &indexCopy);
    });
}

void PVulkanMesh::DrawIndirectInstanced(uint32_t ID)
{
    PROFILE_FUNC_SCOPE("PVulkanMesh::DrawIndirectInstanced")

    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetParallelFramePool()->GetCurrentFrame();

    SUInt64PointerPushConstant PushConstant;
    PushConstant.DeviceAddress = DeviceAddress64;
    PushConstant.ObjectId = ID;

    Material->Bind();

    vkCmdPushConstants(Frame->GetCommandBuffer()->GetVkCommandBuffer(), Material->GraphicsPipeline->GetPipelineLayout()->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SUInt64PointerPushConstant), &PushConstant);
    vkCmdBindIndexBuffer(Frame->GetCommandBuffer()->GetVkCommandBuffer(), IndexBuffer->Buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(Frame->GetCommandBuffer()->GetVkCommandBuffer(), static_cast<size_t>(IndexBuffer->AllocationInfo.size) / sizeof(uint32_t), 1, 0, 0, 0);

    Material->Unbind();
}


void PVulkanMesh::Destroy()
{
    VertexBuffer->Free();
    IndexBuffer->Free();
    StagingBuffer->Free();
    Material->Destroy();
}

IMaterial* PVulkanMesh::GetMaterial() const
{
    return Material;
}


void PVulkanMesh::SetMaterial(IMaterial* NewMaterial)
{
    Material = static_cast<PVulkanMaterial*>(NewMaterial);
//    Material->Init();
}