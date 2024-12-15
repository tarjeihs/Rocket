#pragma once

#include "EnginePCH.h"
#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Types/SharedPtr.h"

struct FVkMesh : public IMesh
{
    SizeType VertexOffset;
    SizeType VertexCount;
    SizeType IndexOffset; 
    SizeType IndexCount;
};

struct FVkMeshBuffer
{
    TSharedPtr<FVkBuffer> VertexBuffer;
    TSharedPtr<FVkBuffer> IndexBuffer;
    TSharedPtr<FVkBuffer> IndirectBuffer;

    TSharedPtr<FVkBuffer> StagingVertexBuffer;
    TSharedPtr<FVkBuffer> StagingIndexBuffer;
    TSharedPtr<FVkBuffer> StagingIndirectBuffer;

    SizeType CurrentVertexOffset = 0;
    SizeType CurrentIndexOffset = 0;
    SizeType CurrentIndirectOffset = 0;

    SizeType StagingVertexOffset = 0;
    SizeType StagingIndexOffset = 0;
    SizeType StagingIndirectOffset = 0;

    TArray<FVkMesh> Metadatas;

    void Initialize()
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

    void AddData(const std::vector<SVertex>& Vertices, const std::vector<uint32> Indices)
    {
        const SizeType VertexBufferSize = Vertices.size() * sizeof(SVertex);
        const SizeType IndexBufferSize = Indices.size() * sizeof(uint32_t);

        FVkMesh Metadata;
        Metadata.VertexOffset = CurrentVertexOffset;
        Metadata.VertexCount = (uint32_t)Vertices.size();
        Metadata.IndexOffset = CurrentIndexOffset;
        Metadata.IndexCount = (uint32_t)Indices.size();

        VkDrawIndexedIndirectCommand IndirectCommand = {};
        IndirectCommand.indexCount = Metadata.IndexCount;
        IndirectCommand.instanceCount = 1;
        IndirectCommand.firstIndex = Metadata.IndexOffset / sizeof(uint32_t);
        IndirectCommand.vertexOffset = Metadata.VertexOffset / sizeof(SVertex);
        IndirectCommand.firstInstance = (uint32_t)Metadatas.GetSize();

        // Write Vertex Data to Staging Buffer
        void* VertexData = nullptr;
        vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingVertexBuffer->Info.Allocation, &VertexData);
        memcpy((char*)VertexData + StagingVertexOffset, Vertices.data(), VertexBufferSize);
        vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingVertexBuffer->Info.Allocation);

        // Write Index Data to Staging Buffer
        void* IndexData = nullptr;
        vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingIndexBuffer->Info.Allocation, &IndexData);
        memcpy((char*)IndexData + StagingIndexOffset, Indices.data(), IndexBufferSize);
        vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingIndexBuffer->Info.Allocation);

        // Write Indirect Command to Staging Buffer
        void* IndirectData = nullptr;
        vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingIndirectBuffer->Info.Allocation, &IndirectData);
        memcpy((char*)IndirectData + StagingIndirectOffset, &IndirectCommand, sizeof(IndirectCommand));
        vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingIndirectBuffer->Info.Allocation);

        GetRHI()->GetSceneRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
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
        Metadatas.Add(Metadata);
    }

    void DrawIndirect(PVulkanCommandBuffer* CommandBuffer)
    {
        vkCmdDrawIndexedIndirect(CommandBuffer->GetVkCommandBuffer(), IndirectBuffer->Info.Handle, 0, Metadatas.GetSize(), sizeof(VkDrawIndexedIndirectCommand));
    }
};