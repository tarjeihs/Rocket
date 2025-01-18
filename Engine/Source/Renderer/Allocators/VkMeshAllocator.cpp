#include "EnginePCH.h"
#include "VkMeshAllocator.h"

#include "meshoptimizer.h"

#include "Renderer/Vulkan/VkMesh.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Types/Vertex.h"
#include "Utils/Profiler.h"

void FVkStaticMeshBuffer::Initialize()
{
    FBufferCreateInfo VertexBufferCreateInfo;
    VertexBufferCreateInfo.Size                     = 512.0 * 1024 * 1024;
    VertexBufferCreateInfo.BufferUsageFlags         = EBufferUsageFlag::Vertex;
    VertexBufferCreateInfo.BufferTransferFlags      = EBufferTransferFlag::Write;
    VertexBufferCreateInfo.MemoryUsageFlags         = EBufferMemoryFlag::Device;
    Info.VertexBuffer = MakeUnique<FVkBuffer>();
    Info.VertexBuffer->Initialize(VertexBufferCreateInfo);

    FBufferCreateInfo IndexBufferCreateInfo;
    IndexBufferCreateInfo.Size                      = 512.0 * 1024 * 1024;
    IndexBufferCreateInfo.BufferUsageFlags          = EBufferUsageFlag::Index;
    IndexBufferCreateInfo.BufferTransferFlags       = EBufferTransferFlag::Write;
    IndexBufferCreateInfo.MemoryUsageFlags          = EBufferMemoryFlag::Device;
    Info.IndexBuffer = MakeUnique<FVkBuffer>();
    Info.IndexBuffer->Initialize(IndexBufferCreateInfo);

    FBufferCreateInfo IndirectBufferCreateInfo;
    IndirectBufferCreateInfo.Size                   = 1.0 * 1024 * 1024;
    IndirectBufferCreateInfo.BufferUsageFlags       = EBufferUsageFlag::Indirect;
    IndirectBufferCreateInfo.BufferTransferFlags    = EBufferTransferFlag::Write;
    IndirectBufferCreateInfo.MemoryUsageFlags       = EBufferMemoryFlag::Device;
    Info.IndirectBuffer = MakeUnique<FVkBuffer>();
    Info.IndirectBuffer->Initialize(IndirectBufferCreateInfo);

    FBufferCreateInfo StagingBufferCreateInfo;
    StagingBufferCreateInfo.BufferUsageFlags        = EBufferUsageFlag::None;
    StagingBufferCreateInfo.BufferTransferFlags     = EBufferTransferFlag::Read | EBufferTransferFlag::Write;
    StagingBufferCreateInfo.MemoryUsageFlags        = EBufferMemoryFlag::Host;

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

void FVkStaticMeshBuffer::Shutdown()
{
    Info.StagingIndexBuffer->Shutdown();
    Info.StagingVertexBuffer->Shutdown();
    Info.StagingIndirectBuffer->Shutdown();
    Info.VertexBuffer->Shutdown();
    Info.IndirectBuffer->Shutdown();
    Info.IndexBuffer->Shutdown();

    Metadata.Clear();
}

void FVkStaticMeshBuffer::AppendInstance(FVkMesh* Mesh)
{
    const TArray<FVertex> Vertices = Mesh->Info.Submeshes.Vertices;
    const TArray<uint32> Indices = Mesh->Info.Submeshes.Indices;
    
    const uint32_t VertexCount = static_cast<uint32_t>(Vertices.GetSize());
    const uint32_t IndexCount  = static_cast<uint32_t>(Indices.GetSize());
    
    const SizeType VertexBufferSize = Vertices.GetSize() * sizeof(FVertex);
    const SizeType IndexBufferSize  = Indices.GetSize() * sizeof(uint32_t);
    
    FStaticMeshBufferInstanceMetadata Instance;
    Instance.VertexOffset  = Info.CurrentVertexOffset;
    Instance.VertexCount   = VertexCount;
    Instance.IndexOffset   = Info.CurrentIndexOffset;
    Instance.IndexCount    = IndexCount;
    
    VkDrawIndexedIndirectCommand IndirectCommand = {};
    IndirectCommand.indexCount   = IndexCount;
    IndirectCommand.instanceCount = 1;
    IndirectCommand.vertexOffset  = (Info.CurrentVertexOffset / sizeof(FVertex));
    IndirectCommand.firstIndex    = (Info.CurrentIndexOffset / sizeof(uint32_t));
    IndirectCommand.firstInstance = Info.Size;

    // Write Vertex Data to Staging Buffer
    void* VertexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation, &VertexData);
    memcpy((char*)VertexData, Vertices.GetData(), VertexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation);

    // Write Index Data to Staging Buffer
    void* IndexData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation, &IndexData);
    memcpy((char*)IndexData, Indices.GetData(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation);

    // Write Indirect Command to Staging Buffer
    void* IndirectData = nullptr;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation, &IndirectData);
    memcpy((char*)IndirectData, &IndirectCommand, sizeof(IndirectCommand));
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation);

    GetRHI()->GetRenderer()->ImmediateSubmit([&](FVkCommandBuffer* CommandBuffer)
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
        break;
    }
}

/*
    Note:
    This approach is slow. Using a staging buffer with immediate submit is slower than having a dynamic mesh (direct GPU access from host(CPU))
    Consider a new approach using a dynamic mesh with direct GPU buffer access
*/
void FVkStaticMeshBuffer::UpdateInstance(FVkMesh* Mesh)
{
    //PROFILE_FUNC_SCOPE("FVkMeshAllocator::UpdateInstance")
//
    //const uint32_t InstanceID = Mesh->Info.Index;
    //FStaticMeshBufferInstanceMetadata& Instance = Metadata[InstanceID];
//
    //const SizeType NewVertexBufferSize = Mesh->Info.Vertices.GetSize() * sizeof(FVertex);
    //const SizeType NewIndexBufferSize  = Mesh->Info.Indices.GetSize()  * sizeof(uint32_t);
//
    //VkDrawIndexedIndirectCommand IndirectCommand = {};
    //IndirectCommand.indexCount    = static_cast<uint32_t>(NewIndexBufferSize / sizeof(uint32_t));
    //IndirectCommand.instanceCount = 1;
    //IndirectCommand.firstIndex    = static_cast<uint32_t>(Instance.IndexOffset / sizeof(uint32_t));
    //IndirectCommand.vertexOffset  = static_cast<int32_t>(Instance.VertexOffset / sizeof(FVertex));
    //IndirectCommand.firstInstance = InstanceID; // or however you're doing instance IDs
//
    //void* VertexDataPtr = nullptr;
    //vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation, &VertexDataPtr);
    //memcpy(VertexDataPtr, Mesh->Info.Vertices.GetData(), NewVertexBufferSize);
    //vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingVertexBuffer->Info.Allocation);
//
    //void* IndexDataPtr = nullptr;
    //vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation, &IndexDataPtr);
    //memcpy(IndexDataPtr, Mesh->Info.Indices.GetData(), NewIndexBufferSize);
    //vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndexBuffer->Info.Allocation);
//
    //void* IndirectDataPtr = nullptr;
    //vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation, &IndirectDataPtr);
    //memcpy(IndirectDataPtr, &IndirectCommand, sizeof(IndirectCommand));
    //vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.StagingIndirectBuffer->Info.Allocation);
//
    //GetRHI()->GetRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
    //{
    //    VkBufferCopy BufferCopyVertex = {};
    //    BufferCopyVertex.srcOffset = 0;
    //    BufferCopyVertex.dstOffset = Instance.VertexOffset;
    //    BufferCopyVertex.size      = NewVertexBufferSize;
    //    vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingVertexBuffer->Info.Handle, Info.VertexBuffer->Info.Handle, 1, &BufferCopyVertex);
//
    //    VkBufferCopy BufferCopyIndex = {};
    //    BufferCopyIndex.srcOffset = 0;
    //    BufferCopyIndex.dstOffset = Instance.IndexOffset;
    //    BufferCopyIndex.size      = NewIndexBufferSize;
    //    vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndexBuffer->Info.Handle, Info.IndexBuffer->Info.Handle, 1, &BufferCopyIndex);
//
    //    VkBufferCopy BufferCopyIndirect = {};
    //    BufferCopyIndirect.srcOffset = 0;
    //    BufferCopyIndirect.dstOffset = sizeof(VkDrawIndexedIndirectCommand) * InstanceID;
    //    BufferCopyIndirect.size      = sizeof(VkDrawIndexedIndirectCommand);
    //    vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), Info.StagingIndirectBuffer->Info.Handle, Info.IndirectBuffer->Info.Handle, 1, &BufferCopyIndirect);
    //});
//
    //Instance.VertexCount = static_cast<uint32_t>(NewVertexBufferSize);
    //Instance.IndexCount  = static_cast<uint32_t>(NewIndexBufferSize);
}

void FVkStaticMeshBuffer::DrawIndexedIndirect(FVkCommandBuffer* CommandBuffer)
{
    vkCmdDrawIndexedIndirect(CommandBuffer->GetVkCommandBuffer(), Info.IndirectBuffer->Info.Handle, 0, Info.Size, sizeof(VkDrawIndexedIndirectCommand));
}