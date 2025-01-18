#pragma once

class FVkCommandBuffer;
class FVkBuffer;
class FVkMesh;

struct FVkStaticMeshBufferInfo
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

struct FStaticMeshBufferInstanceMetadataLOD
{
    SizeType VertexOffset           = 0;
    SizeType VertexCount            = 0;
    SizeType IndexOffset            = 0; 
    SizeType IndexCount             = 0;
};

struct FStaticMeshBufferInstanceMetadata
{
    SizeType VertexOffset           = 0;
    SizeType VertexCount            = 0;
    SizeType IndexOffset            = 0; 
    SizeType IndexCount             = 0;

    TArray<FStaticMeshBufferInstanceMetadataLOD> LOD;
};

struct FVkStaticMeshBuffer
{
    TArray<TOptional<FStaticMeshBufferInstanceMetadata>> Metadata;

    FVkStaticMeshBufferInfo Info;

    void Initialize();
    void Shutdown();

    void AppendInstance(FVkMesh* Mesh);
    void UpdateInstance(FVkMesh* Mesh);

    void DrawIndexedIndirect(FVkCommandBuffer* CommandBuffer);
};