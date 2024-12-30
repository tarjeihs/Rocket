#pragma once

#include "Core/Camera.h"
#include "Math/Transform.h"
#include "Utils/UUID64.h"
#include "Types/String.h"
#include "EngineTypes.h"

class IMesh;
class IMaterial;

struct IComponent {};

struct FUUIDComponent : IComponent
{
    FUUIDComponent() = default;

    FUUID64 UUID;
};

struct FTagComponent : IComponent
{
    FTagComponent() = default;
    FTagComponent(const FString& InTag) : Tag(InTag) {}

    FString Tag;
};

struct FTransformComponent : IComponent
{
    FTransformComponent() = default;
    FTransformComponent(const FTransform& InTransform) : Transform(InTransform) {}

    FTransform Transform;
};

struct FMeshComponent : IComponent
{
    FMeshComponent() = default;
    FMeshComponent(IMesh* InMesh) : Mesh(InMesh) {}

    IMesh* Mesh;
};

struct FMaterialComponent : IComponent
{
    FMaterialComponent() = default;
    FMaterialComponent(IMaterial* InMaterial) : Material(InMaterial) {}

    IMaterial* Material;
};

struct FCameraComponent : IComponent
{
    FCameraComponent() = default;
    FCameraComponent(ICamera* InCamera) : Camera(InCamera) {}

    ICamera* Camera;
};