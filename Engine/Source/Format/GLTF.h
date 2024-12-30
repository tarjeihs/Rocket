#pragma once

#include "Types/Vertex.h"
#include "Types/String.h"
#include "Types/Array.h"

namespace GLTF
{
    void Import(const FString& Path, TArray<FVertex>& Vertices, TArray<uint32_t>& Indices);
}