#pragma once

#include <vulkan/vulkan_core.h>

#include "VulkanMemory.h"
#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

class PVulkanMaterial;

struct SDrawQueue
{
    std::function<void()> Draw;
};

class PVulkanMesh : public IMesh
{
public:
    virtual void Init() override;
    virtual void Cleanup() override;

    virtual void CreateMesh(std::span<SVertex> Vertices, std::span<uint32_t> Indices) override;
    virtual void Draw(const STransform& Transform) override;
    virtual void Destroy() override;

    virtual void Serialize(SBlob& Blob) override;
    virtual void Deserialize(SBlob& Blob) override;

    void BeginFrame();
    void EndFrame();

    void SetMaterial(const std::shared_ptr<PVulkanMaterial>& NewMaterial);

protected:
    std::vector<std::function<void()>> SubmitData;
    std::vector<SShaderStorageBufferObject> BufferData;

private:
    std::shared_ptr<PVulkanMaterial> Material;
    std::unique_ptr<SVulkanBuffer> VertexBuffer;
    std::unique_ptr<SVulkanBuffer> IndexBuffer;
    std::unique_ptr<SVulkanBuffer> StagingBuffer;

    VkDeviceAddress DeviceAddress64;
};
