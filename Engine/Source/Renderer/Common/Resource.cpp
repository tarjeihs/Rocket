#include "EnginePCH.h"
#include "Resource.h"

#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanShader.h"

template<typename TExplicit>
TExplicit* NewObject()
{
    using Type = typename TResource<TExplicit>::Type;
    static_assert(std::is_base_of<TExplicit, Type>::value, "Type must derive from TExplicit.");
    return new Type();
}

template IMesh* NewObject<IMesh>();
template IShader* NewObject<IShader>();
template IMaterial* NewObject<IMaterial>();