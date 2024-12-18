#pragma once

#include <vector>

#include "Renderer/RHI.h"

class PVulkanInstance;
class PVulkanDevice;
class PVulkanMemory;
class PVulkanForwardRenderer;
class FVulkanRenderer;
class PVulkanAllocator;

class PVulkanRHI : public IRHI
{
public:
    struct FExtensionFamily
    {
        std::vector<const char*> ValidationLayerExtensions = {};
        std::vector<const char*> InstanceExtensions = {};
        std::vector<const char*> PhysicalDeviceExtensions = { "VK_KHR_swapchain" };
    } ExtensionFamily;

    // IRHI interface
    virtual void Init() final override;
    virtual void Shutdown() final override;
    virtual void Resize() final override;
    virtual void Render() final override;

    inline PVulkanInstance* GetInstance() const;
    inline PVulkanDevice* GetDevice() const;
    inline PVulkanAllocator* GetAllocator() const;
    inline FVulkanRenderer* GetRenderer() const;

private:
    PVulkanInstance* Instance;
    PVulkanDevice* Device;
    PVulkanAllocator* Allocator;
    FVulkanRenderer* Renderer;
};

inline PVulkanInstance* PVulkanRHI::GetInstance() const
{
	return Instance;
}

inline PVulkanDevice* PVulkanRHI::GetDevice() const
{
	return Device;
}

inline PVulkanAllocator* PVulkanRHI::GetAllocator() const
{
	return Allocator;
}

inline FVulkanRenderer* PVulkanRHI::GetRenderer() const
{
	return Renderer;
}