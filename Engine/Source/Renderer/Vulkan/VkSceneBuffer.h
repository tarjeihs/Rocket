#pragma once

#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Types/SharedPtr.h"
#include "Types/Vertex.h"

struct FInstanceMetadata
{
    SizeType VertexOffset;
    SizeType VertexCount;
    SizeType IndexOffset; 
    SizeType IndexCount;
};

class FVkSceneInstanceManager
{
public:
    TArray<FInstanceMetadata> Metadata;

    SizeType CurrentVertexOffset = 0;
    SizeType CurrentIndexOffset = 0;
    SizeType CurrentIndirectOffset = 0;

    SizeType StagingVertexOffset = 0;
    SizeType StagingIndexOffset = 0;
    SizeType StagingIndirectOffset = 0;

    TSharedPtr<FVkBuffer> VertexBuffer;
    TSharedPtr<FVkBuffer> IndexBuffer;
    TSharedPtr<FVkBuffer> IndirectBuffer;

    TSharedPtr<FVkBuffer> StagingVertexBuffer;
    TSharedPtr<FVkBuffer> StagingIndexBuffer;
    TSharedPtr<FVkBuffer> StagingIndirectBuffer;

    void Initialize();
    void Shutdown();
    void DrawIndexedIndirect(PVulkanCommandBuffer* CommandBuffer);
    uint32 AddObject(const TArray<FVertex>& Vertices, const TArray<uint32>& Indices);
};