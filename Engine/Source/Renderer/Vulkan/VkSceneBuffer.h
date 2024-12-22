#pragma once

#include "Renderer/Common/SceneBuffer.h"
#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Types/SharedPtr.h"

class FVkSceneBuffer : public ISceneBuffer
{
public:
    TSharedPtr<FVkBuffer> VertexBuffer;
    TSharedPtr<FVkBuffer> IndexBuffer;
    TSharedPtr<FVkBuffer> IndirectBuffer;

    TSharedPtr<FVkBuffer> StagingVertexBuffer;
    TSharedPtr<FVkBuffer> StagingIndexBuffer;
    TSharedPtr<FVkBuffer> StagingIndirectBuffer;

    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual uint32 AddInstance(const std::vector<FVertex>& Vertices, const std::vector<uint32> Indices) override;
    void DrawIndexedIndirect(PVulkanCommandBuffer* CommandBuffer);
};