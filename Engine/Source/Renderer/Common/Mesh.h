#pragma once

#include <glm/glm.hpp>

#include "Renderer/Common/Material.h"

struct STransform;



struct SVertex
{
    SVertex()
    {
        static_assert(sizeof(SVertex) == 16, "SVertex size should be 16 bytes.");

        Position = glm::vec3(0.0f);
    }

    alignas(16) glm::vec3 Position;
};

struct SMeshBinaryData
{
    std::vector<SVertex> Vertices;
    std::vector<uint32_t> Indices;
};

struct SMeshSettings
{
    IMaterial* Material;
};

enum class EVisibilityMode
{
    None = 0,
    Visible,
    Hidden
};

class IMesh
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(const SMeshBinaryData& MeshBinaryObject) = 0;
    virtual void CreateDynamicMesh(const SMeshBinaryData& MeshBinaryObject) = 0;
    virtual void DrawIndirectInstanced(uint32_t ID) = 0;
    virtual void Destroy() = 0;

    virtual void UpdateDynamicMesh(const SMeshBinaryData& MeshData) = 0;
    
    virtual IMaterial* GetMaterial() const = 0;
    virtual void SetMaterial(IMaterial* NewMaterial) = 0;

    virtual void SetVisibility(EVisibilityMode Mode) = 0;
    virtual EVisibilityMode GetVisibility() const = 0;
};
