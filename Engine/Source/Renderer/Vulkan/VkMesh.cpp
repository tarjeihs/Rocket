#include "EnginePCH.h"
#include "VkMesh.h"

#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Allocators/VkMeshAllocator.h"

void FVkMesh::CreateMesh(FGeometry& Geometry)
{
    Info.Vertices = Geometry.Vertices;
    Info.Indices = Geometry.Indices;

    GetRHI()->GetRenderer()->GetMeshAllocator()->AppendInstance(this);
}

void FVkMesh::UpdateMesh(FGeometry& Geometry)
{
    Info.Vertices = Geometry.Vertices;
    Info.Indices = Geometry.Indices;

    GetRHI()->GetRenderer()->GetMeshAllocator()->UpdateInstance(this);
}