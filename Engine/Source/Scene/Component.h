#pragma once

#include "Math/Transform.h"
#include "Utils/UUID64.h"
#include "Renderer/Common/Mesh.h"

struct IComponent {};

struct STagComponent : IComponent
{
    std::string Tag;
};

struct STransformComponent : IComponent
{
    STransformComponent() = default;
    STransformComponent(const STransform& InTransform) : Transform(InTransform) {}

    STransform Transform;
};

struct SUUIDComponent : IComponent
{
    SUUID64 UUID;
};

struct SMeshComponent : IComponent
{
    SMeshComponent() = default;
    SMeshComponent(IMesh* InMesh) : Mesh(InMesh) {}

    IMesh* Mesh;
};

struct SInstancedMeshComponent : IComponent
{
    SInstancedMeshComponent() = default;
    SInstancedMeshComponent(IMesh* InMesh) : Mesh(InMesh) {}

    IMesh* Mesh;
};