#include "EnginePCH.h"
#include "RHIAPI.h"

#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanShader.h"

template<>
struct TRHIAPI<IMesh>
{
#if RK_RHI == VULKAN
    using Type = FVkMesh;
#endif
};

template<>
struct TRHIAPI<IShader>
{
#if RK_RHI == VULKAN
    using Type = FVkShader;
#endif
};

template<>
struct TRHIAPI<IMaterial>
{
#if RK_RHI == VULKAN
    using Type = PVulkanMaterial;
#endif
};

template<typename TObject>
TObject* NewObject()
{
    using Type = typename TRHIAPI<TObject>::Type;
    static_assert(std::is_base_of<TObject, Type>::value, "Type must derive from TObject.");
    return new Type();
}

template IMesh* NewObject<IMesh>();
template IShader* NewObject<IShader>();
template IMaterial* NewObject<IMaterial>();