#include "EnginePCH.h"
#include "RHIAPI.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VkSceneBuffer.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "SceneBuffer.h"

template<>
struct TRHIAPI<ISceneBuffer>
{
#if RK_RHI == VULKAN
    using Type = FVkSceneBuffer;
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

template ISceneBuffer* NewObject<ISceneBuffer>();
template IShader* NewObject<IShader>();
template IMaterial* NewObject<IMaterial>();

uint32 AddInstance(std::vector<FVertex> Vertices, std::vector<uint32> Indices)
{
    uint32 InstanceID = 0;
#if RK_RHI == VULKAN
    GetRHI()->GetRenderer()->GetSceneBuffer()->AddInstance(Vertices, Indices);
    return InstanceID;
#endif
}