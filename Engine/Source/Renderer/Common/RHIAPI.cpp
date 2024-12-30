#include "EnginePCH.h"
#include "RHIAPI.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanTexture2D.h"
#include "Renderer/Vulkan/VkMesh.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Format/GLTF.h"
#include "Types/Vertex.h"

template<>
struct TRHIAPI<IMesh>
{
#if RK_RHI == VULKAN
    using Type = FVkMesh;
#endif
};

template<>
struct TRHIAPI<ITexture2D>
{
#if RK_RHI == VULKAN
    using Type = FVkTexture2D;
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
template ITexture2D* NewObject<ITexture2D>();