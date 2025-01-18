#pragma once

#include <vector>

#include "Renderer/RHI.h"

class FVkInstance;
class FVkDevice;
class PVulkanMemory;
class FVkAllocator;
class FVkRenderer;

class PVulkanRHI : public IRHI
{
public:
    struct FExtensionFamily
    {
        std::vector<const char*> ValidationLayerExtensions = {};
        std::vector<const char*> InstanceExtensions = { "VK_EXT_swapchain_colorspace" };
        std::vector<const char*> PhysicalDeviceExtensions = { "VK_KHR_swapchain" };
    } ExtensionFamily;

    // IRHI interface
    virtual void Init() final override;
    virtual void Shutdown() final override;
    virtual void Resize() final override;
    virtual void Render() final override;

    inline FVkInstance* GetInstance() const;
    inline FVkDevice* GetDevice() const;
    inline FVkAllocator* GetAllocator() const;
    inline FVkRenderer* GetRenderer() const;

private:
    FVkInstance* Instance;
    FVkDevice* Device;
    FVkAllocator* Allocator;
    FVkRenderer* Renderer;
};

inline FVkInstance* PVulkanRHI::GetInstance() const
{
	return Instance;
}

inline FVkDevice* PVulkanRHI::GetDevice() const
{
	return Device;
}

inline FVkAllocator* PVulkanRHI::GetAllocator() const
{
	return Allocator;
}

inline FVkRenderer* PVulkanRHI::GetRenderer() const
{
	return Renderer;
}