#pragma once

class IMesh;
class IMaterial;
class IShader;

template<typename TExplicit>
struct TResource;

template<>
struct TResource<IMesh>
{
#if RK_RHI == VULKAN
    using Type = class PVulkanMesh;
#endif
};

template<>
struct TResource<IShader>
{
#if RK_RHI == VULKAN
    using Type = class PVulkanShader;
#endif
};

template<>
struct TResource<IMaterial>
{
#if RK_RHI == VULKAN
    using Type = class PVulkanMaterial;
#endif
};

template<typename TExplicit>
TExplicit* NewObject();