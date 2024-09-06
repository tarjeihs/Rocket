#pragma once

#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanMemory.h"

class PVulkanMaterial;
class SVulkanBuffer;

typedef uint64_t VkDeviceAddress;

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

    virtual void ApplyMaterial(IMaterial* NewMaterial) override;

private:
    PVulkanMaterial* Material;
    SVulkanBuffer* VertexBuffer;
    SVulkanBuffer* IndexBuffer;
    SVulkanBuffer* StagingBuffer;

    // TODO: Create a small struct wrapper for device addr in VulkanMemory.h
    VkDeviceAddress DeviceAddress64;
};
