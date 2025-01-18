#pragma once

#include "Types/Vertex.h"
#include "Types/Array.h"
#include "Types/String.h"
#include "Math/Transform.h"

struct FMaterial
{
    int32 AlbedoID = INDEX_Invalid;
    int32 NormalID = INDEX_Invalid;
    int32 RoughnessID = INDEX_Invalid;
    int32 MetallicID = INDEX_Invalid;
};

struct FSubmesh
{
    FString Name;
    FMaterial Material;
    FTransform Transform;

    TArray<FVertex> Vertices;
    TArray<uint32_t> Indices;
};

struct FPrefab
{
    TArray<FSubmesh> Submeshes;
};