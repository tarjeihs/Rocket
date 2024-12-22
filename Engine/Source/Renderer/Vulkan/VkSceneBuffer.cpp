#include "EnginePCH.h"
#include "VkSceneBuffer.h"

#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanCommand.h"

void FVkSceneBuffer::Initialize()
{
    FVkBufferCreateInfo VertexBufferCreateInfo;
    VertexBufferCreateInfo.Size = 1024 * MiB;
    VertexBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VertexBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    VertexBuffer = MakeShared<FVkBuffer>();
    VertexBuffer->Initialize(VertexBufferCreateInfo);

    FVkBufferCreateInfo IndexBufferCreateInfo;
    IndexBufferCreateInfo.Size = 1024 * MiB;
    IndexBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    IndexBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    IndexBuffer = MakeShared<FVkBuffer>();
    IndexBuffer->Initialize(IndexBufferCreateInfo);

    FVkBufferCreateInfo IndirectBufferCreateInfo;
    IndirectBufferCreateInfo.Size = 1024 * MiB;
    IndirectBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    IndirectBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    IndirectBuffer = MakeShared<FVkBuffer>();
    IndirectBuffer->Initialize(IndirectBufferCreateInfo);

    FVkBufferCreateInfo StagingBufferCreateInfo;
    StagingBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    StagingBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_CPU_ONLY;

    StagingBufferCreateInfo.Size = 1024 * MiB;
    StagingVertexBuffer = MakeShared<FVkBuffer>();
    StagingVertexBuffer->Initialize(StagingBufferCreateInfo);

    StagingBufferCreateInfo.Size = 1024 * MiB;
    StagingIndexBuffer = MakeShared<FVkBuffer>();
    StagingIndexBuffer->Initialize(StagingBufferCreateInfo);

    StagingBufferCreateInfo.Size = 1024 * MiB;
    StagingIndirectBuffer = MakeShared<FVkBuffer>();
    StagingIndirectBuffer->Initialize(StagingBufferCreateInfo);
}

void FVkSceneBuffer::Shutdown()
{
    StagingIndexBuffer->Free();
    StagingVertexBuffer->Free();
    StagingIndirectBuffer->Free();
    VertexBuffer->Free();
    IndirectBuffer->Free();
    IndexBuffer->Free();
}

uint32 FVkSceneBuffer::AddInstance(const std::vector<FVertex>& Vertices, const std::vector<uint32> Indices)
{
    const SizeType VertexBufferSize = Vertices.size() * sizeof(FVertex);
    const SizeType IndexBufferSize = Indices.size() * sizeof(uint32_t);

    FInstanceMetadata Instance;
    Instance.VertexOffset = CurrentVertexOffset;
    Instance.VertexCount = (uint32_t)Vertices.size();
    Instance.IndexOffset = CurrentIndexOffset;
    Instance.IndexCount = (uint32_t)Indices.size();

    VkDrawIndexedIndirectCommand IndirectCommand = {};
    IndirectCommand.indexCount = Instance.IndexCount;
    IndirectCommand.instanceCount = 1;
    IndirectCommand.firstIndex = Instance.IndexOffset / sizeof(uint32_t);
    IndirectCommand.vertexOffset = Instance.VertexOffset / sizeof(FVertex);
    IndirectCommand.firstInstance = (uint32_t)Metadata.GetSize();

    // Write Vertex Data to Staging Buffer
    void* VertexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingVertexBuffer->Info.Allocation, &VertexData);
    memcpy((char*)VertexData + StagingVertexOffset, Vertices.data(), VertexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingVertexBuffer->Info.Allocation);

    // Write Index Data to Staging Buffer
    void* IndexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingIndexBuffer->Info.Allocation, &IndexData);
    memcpy((char*)IndexData + StagingIndexOffset, Indices.data(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingIndexBuffer->Info.Allocation);

    // Write Indirect Command to Staging Buffer
    void* IndirectData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingIndirectBuffer->Info.Allocation, &IndirectData);
    memcpy((char*)IndirectData + StagingIndirectOffset, &IndirectCommand, sizeof(IndirectCommand));
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), StagingIndirectBuffer->Info.Allocation);

    GetRHI()->GetRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        VkBufferCopy VertexBufferCopy = {};
        VertexBufferCopy.srcOffset = StagingVertexOffset;
        VertexBufferCopy.dstOffset = CurrentVertexOffset;
        VertexBufferCopy.size = VertexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingVertexBuffer->Info.Handle, VertexBuffer->Info.Handle, 1, &VertexBufferCopy);

        VkBufferCopy IndexBufferCopy = {};
        IndexBufferCopy.srcOffset = StagingIndexOffset;
        IndexBufferCopy.dstOffset = CurrentIndexOffset;
        IndexBufferCopy.size = IndexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingIndexBuffer->Info.Handle, IndexBuffer->Info.Handle, 1, &IndexBufferCopy);

        VkBufferCopy IndirectBufferCopy = {};
        IndirectBufferCopy.srcOffset = StagingIndirectOffset;
        IndirectBufferCopy.dstOffset = CurrentIndirectOffset;
        IndirectBufferCopy.size = sizeof(VkDrawIndexedIndirectCommand);
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingIndirectBuffer->Info.Handle, IndirectBuffer->Info.Handle, 1, &IndirectBufferCopy);
    });

    // Update GPU offsets
    CurrentVertexOffset += VertexBufferSize;
    CurrentIndexOffset += IndexBufferSize;
    CurrentIndirectOffset += sizeof(VkDrawIndexedIndirectCommand);

    // Update staging offsets
    StagingVertexOffset += VertexBufferSize;
    StagingIndexOffset += IndexBufferSize;
    StagingIndirectOffset += sizeof(VkDrawIndexedIndirectCommand);

    // Add metadata entry
    Metadata.Add(Instance);

    return Metadata.GetSize();
}

void FVkSceneBuffer::DrawIndexedIndirect(PVulkanCommandBuffer* CommandBuffer)
{
    vkCmdDrawIndexedIndirect(CommandBuffer->GetVkCommandBuffer(), IndirectBuffer->Info.Handle, 0, Metadata.GetSize(), sizeof(VkDrawIndexedIndirectCommand));
}