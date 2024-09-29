#pragma once

#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanMemory.h"

class PVulkanMaterial;
class PVulkanBuffer;

typedef uint64_t VkDeviceAddress;

class PVulkanMesh : public IMesh
{
public:
    virtual void Init() override;
    virtual void Cleanup() override;

    virtual void CreateMesh(std::span<SVertex> Vertices, std::span<uint32_t> Indices) override;
    virtual void DrawIndirectInstanced(uint32_t ID) override;
    virtual void Destroy() override;

    virtual void Serialize(SBlob& Blob) override;
    virtual void Deserialize(SBlob& Blob) override;

    virtual void ApplyMaterial(IMaterial* NewMaterial) override;

private:
    PVulkanMaterial* Material;
    PVulkanBuffer* VertexBuffer;
    PVulkanBuffer* IndexBuffer;
    PVulkanBuffer* StagingBuffer;

    // TODO: Create a small struct wrapper for device addr in VulkanMemory.h
    VkDeviceAddress DeviceAddress64;
};
