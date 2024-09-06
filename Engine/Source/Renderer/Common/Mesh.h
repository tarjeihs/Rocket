#pragma once

#include <span>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "Asset/Asset.h"
#include "Math/Transform.h"

struct STransform;

struct SVertex
{
    glm::vec3 Position;
    glm::vec2 TexCoord;
    glm::vec3 Normal;
    glm::vec4 Color;
};

class IMesh : public IAssetMarshalInterface
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(std::span<SVertex> Vertices, std::span<uint32_t> Indices) = 0;
    virtual void Draw(const STransform& Transform) = 0;
    virtual void Destroy() = 0;
    virtual void ApplyMaterial(class IMaterial* NewMaterial) = 0;
};
