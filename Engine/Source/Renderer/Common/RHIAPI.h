#pragma once

class IMesh;
class IMaterial;
class IShader;

template<typename TExplicit>
struct TRHIAPI;

template<>
struct TRHIAPI<IMesh>
{
#if RK_RHI == VULKAN
    using Type = class FVkMesh;
#endif
};

template<>
struct TRHIAPI<IShader>
{
#if RK_RHI == VULKAN
    using Type = class PVulkanShader;
#endif
};

template<>
struct TRHIAPI<IMaterial>
{
#if RK_RHI == VULKAN
    using Type = class PVulkanMaterial;
#endif
};

template<typename TObject>
TObject* NewObject();