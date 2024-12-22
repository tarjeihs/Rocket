#pragma once

#include "Math/Transform.h"
#include "Utils/UUID64.h"
#include "Renderer/Common/Mesh.h"

struct IComponent {};

struct FTagComponent : IComponent
{
    std::string Tag;
};

struct FTransformComponent : IComponent
{
    FTransformComponent() = default;
    FTransformComponent(const STransform& InTransform) : Transform(InTransform) {}

    STransform Transform;
};

struct FUUIDComponent : IComponent
{
    SUUID64 UUID;
};

struct FMeshComponent : IComponent
{
    FMeshComponent() = default;
    FMeshComponent(IMesh* InMesh) : Mesh(InMesh) {}

    IMesh* Mesh;
};

struct FInstancedMeshComponent : IComponent
{
    FInstancedMeshComponent() = default;
    FInstancedMeshComponent(IMesh* InMesh) : Mesh(InMesh) {}

    IMesh* Mesh;
};