#pragma once

#include "Renderer/Common/Mesh.h"

class PVulkanMaterial;
class PVulkanBuffer;

typedef uint64_t VkDeviceAddress;

class PVulkanMesh : public IMesh
{
public:
    virtual void CreateMesh(const SMeshBinaryData& MeshBinaryObject) override;
    virtual void CreateDynamicMesh(const SMeshBinaryData& MeshBinaryObject) override;
    virtual void DrawIndirectInstanced(uint32_t ID) override;
    virtual void Destroy() override;

    virtual void UpdateDynamicMesh(const SMeshBinaryData& MeshData) override;

    virtual IMaterial* GetMaterial() const override;
    virtual void SetMaterial(IMaterial* NewMaterial) override;

    virtual void SetVisibility(EVisibilityMode Mode) override;
    virtual EVisibilityMode GetVisibility() const override;

private:
    PVulkanMaterial* Material;
    PVulkanBuffer* VertexBuffer;
    PVulkanBuffer* IndexBuffer;
    PVulkanBuffer* StagingBuffer;

    // TODO: Create a small struct wrapper for device addr in VulkanMemory.h
    VkDeviceAddress DeviceAddress64;

    EVisibilityMode VisibilityMode;
};
