#include "EnginePCH.h"
#include "VkMeshAllocator.h"

#include "Renderer/Vulkan/VkMesh.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Types/Vertex.h"
#include "Utils/Profiler.h"

void FVkMeshAllocator::Initialize()
{
    FVkBufferCreateInfo VertexBufferCreateInfo;
    VertexBufferCreateInfo.Size = 2048.0 * 1024 * 1024;
    VertexBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VertexBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    Info.VertexBuffer = MakeUnique<FVkBuffer>();
    Info.VertexBuffer->Initialize(VertexBufferCreateInfo);

    FVkBufferCreateInfo IndexBufferCreateInfo;
    IndexBufferCreateInfo.Size = 2048.0 * 1024 * 1024;
    IndexBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    IndexBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    Info.IndexBuffer = MakeUnique<FVkBuffer>();
    Info.IndexBuffer->Initialize(IndexBufferCreateInfo);

    FVkBufferCreateInfo IndirectBufferCreateInfo;
    IndirectBufferCreateInfo.Size = 1.0 * 1024 * 1024;
    IndirectBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    IndirectBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_GPU_ONLY;
    Info.IndirectBuffer = MakeUnique<FVkBuffer>();
    Info.IndirectBuffer->Initialize(IndirectBufferCreateInfo);

    FVkBufferCreateInfo StagingBufferCreateInfo;
    StagingBufferCreateInfo.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    StagingBufferCreateInfo.MemoryUsageFlags = VMA_MEMORY_USAGE_CPU_ONLY;

    StagingBufferCreateInfo.Size = 128.0 * 1024 * 1024;
    Info.StagingVertexBuffer = MakeUnique<FVkBuffer>();
    Info.StagingVertexBuffer->Initialize(StagingBufferCreateInfo);

    StagingBufferCreateInfo.Size = 64.0 * 1024 * 1024;
    Info.StagingIndexBuffer = MakeUnique<FVkBuffer>();
    Info.StagingIndexBuffer->Initialize(StagingBufferCreateInfo);

    StagingBufferCreateInfo.Size = 1.0 * 1024 * 1024;
    Info.StagingIndirectBuffer = MakeUnique<FVkBuffer>();
    Info.StagingIndirectBuffer->Initialize(StagingBufferCreateInfo);

    Metadata.Resize(1024 * 1024);
}

void FVkMeshAllocator::Shutdown()
{
    Info.StagingIndexBuffer->Free();
    Info.StagingVertexBuffer->Free();
    Info.StagingIndirectBuffer->Free();
    Info.VertexBuffer->Free();
    Info.IndirectBuffer->Free();
    Info.IndexBuffer->Free();

    Metadata.Clear();
}

void FVkMeshAllocator::AppendInstance(FVkMesh* Mesh)
{
    const SizeType VertexBufferSize = Mesh->Info.Vertices.GetSize() * sizeof(FVertex);
    const SizeType IndexBufferSize = Mesh->Info.Indices.GetSize() * sizeof(uint32_t);

    if (VertexBufferSize == 0 || IndexBufferSize == 0) return;

    FInstanceMetadata Instance;
    Instance.VertexOffset = Info.CurrentVertexOffset;
    Instance.VertexCount = (uint32_t)VertexBufferSize;
    Instance.IndexOffset = Info.CurrentIndexOffset;
    Instance.IndexCount = (uint32_t)IndexBufferSize;

    VkDrawIndexedIndirectCommand IndirectCommand = {};
    IndirectCommand.indexCount = Instance.IndexCount;
    IndirectCommand.instanceCount = 1;
    IndirectCommand.firstIndex = Instance.IndexOffset / sizeof(uint32_t);
    IndirectCommand.vertexOffset = Instance.VertexOffset / sizeof(FVertex);
    IndirectCommand.firstInstance = Info.Size;

    // Write Vertex Data to Staging Buffer
    void* VertexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation, &VertexData);
    memcpy((char*)VertexData, Mesh->Info.Vertices.GetData(), VertexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation);

    // Write Index Data to Staging Buffer
    void* IndexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation, &IndexData);
    memcpy((char*)IndexData, Mesh->Info.Indices.GetData(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation);

    // Write Indirect Command to Staging Buffer
    void* IndirectData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation, &IndirectData);
    memcpy((char*)IndirectData, &IndirectCommand, sizeof(IndirectCommand));
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation);

    GetRHI()->GetRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        VkBufferCopy VertexBufferCopy = {};
        VertexBufferCopy.srcOffset = 0;
        VertexBufferCopy.dstOffset = Info.CurrentVertexOffset;
        VertexBufferCopy.size = VertexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingVertexBuffer->Info.Handle, Info.VertexBuffer->Info.Handle, 1, &VertexBufferCopy);

        VkBufferCopy IndexBufferCopy = {};
        IndexBufferCopy.srcOffset = 0;
        IndexBufferCopy.dstOffset = Info.CurrentIndexOffset;
        IndexBufferCopy.size = IndexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndexBuffer->Info.Handle, Info.IndexBuffer->Info.Handle, 1, &IndexBufferCopy);

        VkBufferCopy IndirectBufferCopy = {};
        IndirectBufferCopy.srcOffset = 0;
        IndirectBufferCopy.dstOffset = Info.CurrentIndirectOffset;
        IndirectBufferCopy.size = sizeof(IndirectCommand);
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndirectBuffer->Info.Handle, Info.IndirectBuffer->Info.Handle, 1, &IndirectBufferCopy);
    });

    Info.CurrentVertexOffset += VertexBufferSize;
    Info.CurrentIndexOffset += IndexBufferSize;
    Info.CurrentIndirectOffset += sizeof(IndirectCommand);
    Info.Size++;
    
    for (uint32 Index = 0; Index < Metadata.GetSize(); ++Index)
    {
        if (Metadata[Index].IsValid())
        {
            continue;
        }
        
        Metadata[Index] = Instance;
        Mesh->Info.Index = Index;
        break;
    }
}

void FVkMeshAllocator::UpdateInstance(FVkMesh* Mesh)
{
    PROFILE_FUNC_SCOPE("FVkMeshAllocator::UpdateInstance")

    const uint32_t InstanceID = Mesh->Info.Index;
    FInstanceMetadata& Instance = Metadata[InstanceID];

    const SizeType NewVertexBufferSize = Mesh->Info.Vertices.GetSize() * sizeof(FVertex);
    const SizeType NewIndexBufferSize  = Mesh->Info.Indices.GetSize()  * sizeof(uint32_t);

    VkDrawIndexedIndirectCommand IndirectCommand = {};
    IndirectCommand.indexCount    = static_cast<uint32_t>(NewIndexBufferSize / sizeof(uint32_t));
    IndirectCommand.instanceCount = 1;
    IndirectCommand.firstIndex    = static_cast<uint32_t>(Instance.IndexOffset / sizeof(uint32_t));
    IndirectCommand.vertexOffset  = static_cast<int32_t>(Instance.VertexOffset / sizeof(FVertex));
    IndirectCommand.firstInstance = InstanceID; // or however you're doing instance IDs

    void* VertexDataPtr = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation, &VertexDataPtr);
    memcpy(VertexDataPtr, Mesh->Info.Vertices.GetData(), NewVertexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation);

    void* IndexDataPtr = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation, &IndexDataPtr);
    memcpy(IndexDataPtr, Mesh->Info.Indices.GetData(), NewIndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation);

    void* IndirectDataPtr = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation, &IndirectDataPtr);
    memcpy(IndirectDataPtr, &IndirectCommand, sizeof(IndirectCommand));
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation);

    GetRHI()->GetRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    {
        VkBufferCopy BufferCopyVertex = {};
        BufferCopyVertex.srcOffset = 0;                     // Start at 0 in the staging buffer
        BufferCopyVertex.dstOffset = Instance.VertexOffset; // Suballocation offset in GPU buffer
        BufferCopyVertex.size      = NewVertexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingVertexBuffer->Info.Handle, Info.VertexBuffer->Info.Handle, 1, &BufferCopyVertex);

        VkBufferCopy BufferCopyIndex = {};
        BufferCopyIndex.srcOffset = 0;
        BufferCopyIndex.dstOffset = Instance.IndexOffset;
        BufferCopyIndex.size      = NewIndexBufferSize;
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndexBuffer->Info.Handle, Info.IndexBuffer->Info.Handle, 1, &BufferCopyIndex);

        VkBufferCopy BufferCopyIndirect = {};
        BufferCopyIndirect.srcOffset = 0;
        BufferCopyIndirect.dstOffset = sizeof(VkDrawIndexedIndirectCommand) * InstanceID;
        BufferCopyIndirect.size      = sizeof(VkDrawIndexedIndirectCommand);
        vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndirectBuffer->Info.Handle, Info.IndirectBuffer->Info.Handle, 1, &BufferCopyIndirect);
    });

    Instance.VertexCount = static_cast<uint32_t>(NewVertexBufferSize);
    Instance.IndexCount  = static_cast<uint32_t>(NewIndexBufferSize);
}

void FVkMeshAllocator::DrawIndexedIndirect(PVulkanCommandBuffer* CommandBuffer)
{
    vkCmdDrawIndexedIndirect(CommandBuffer->GetVkCommandBuffer(), Info.IndirectBuffer->Info.Handle, 0, Info.Size, sizeof(VkDrawIndexedIndirectCommand));
}