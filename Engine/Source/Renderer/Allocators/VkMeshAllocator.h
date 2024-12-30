#pragma once

class PVulkanCommandBuffer;
class FVkBuffer;
class FVkMesh;

struct FVkMeshAllocator
{
    struct FVkMeshAllocatorInfo
    {
        TUniquePtr<FVkBuffer> VertexBuffer;
        TUniquePtr<FVkBuffer> IndexBuffer;
        TUniquePtr<FVkBuffer> IndirectBuffer;

        TUniquePtr<FVkBuffer> StagingVertexBuffer;
        TUniquePtr<FVkBuffer> StagingIndexBuffer;
        TUniquePtr<FVkBuffer> StagingIndirectBuffer;
        
        SizeType CurrentVertexOffset    = 0;
        SizeType CurrentIndexOffset     = 0;
        SizeType CurrentIndirectOffset  = 0;

        SizeType StagingVertexOffset    = 0;
        SizeType StagingIndexOffset     = 0;
        SizeType StagingIndirectOffset  = 0;

        SizeType Size = 0;
    };

    struct FInstanceMetadata
    {
        FInstanceMetadata() = default;

        SizeType VertexOffset           = 0;
        SizeType VertexCount            = 0;
        SizeType IndexOffset            = 0; 
        SizeType IndexCount             = 0;
        SizeType IndirectOffset         = 0;
    };

    TArray<TOptional<FVkMeshAllocator::FInstanceMetadata>> Metadata;

    FVkMeshAllocatorInfo Info;

    void Initialize();
    void Shutdown();

    void AppendInstance(FVkMesh* Mesh);
    void UpdateInstance(FVkMesh* Mesh);
    void RemoveInstance(FVkMesh* Mesh);

    void DrawIndexedIndirect(PVulkanCommandBuffer* CommandBuffer);
};