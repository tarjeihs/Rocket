#pragma once

struct FInstanceMetadata
{
    SizeType VertexOffset;
    SizeType VertexCount;
    SizeType IndexOffset; 
    SizeType IndexCount;
};

class ISceneBuffer
{
public:
    SizeType CurrentVertexOffset = 0;
    SizeType CurrentIndexOffset = 0;
    SizeType CurrentIndirectOffset = 0;

    SizeType StagingVertexOffset = 0;
    SizeType StagingIndexOffset = 0;
    SizeType StagingIndirectOffset = 0;

    TArray<FInstanceMetadata> Metadata;

    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual uint32 AddInstance(const std::vector<FVertex>& Vertices, const std::vector<uint32> Indices) = 0;
};