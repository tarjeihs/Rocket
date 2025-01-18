#include "EnginePCH.h"
#include "VkMesh.h"

#include "Renderer/Common/Geometry.h"
#include "Renderer/Vulkan/VkRenderer.h"

void FVkMesh::CreateMesh(FSubmesh& Submesh)
{
    Info.Submeshes = Submesh;

    GetRHI()->GetRenderer()->GetMeshAllocator()->AppendInstance(this);
}

void FVkMesh::UpdateMesh(FSubmesh& Submesh)
{
//    GetRHI()->GetRenderer()->GetMeshAllocator()->UpdateInstance(this);
}