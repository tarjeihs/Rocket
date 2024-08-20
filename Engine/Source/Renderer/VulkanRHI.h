#pragma once

#include "RHI.h"

#include <vector>

class PVulkanInstance;
class PVulkanDevice;
class PVulkanMemory;
class PVulkanSceneRenderer;

class PVulkanRHI : public IRHI
{
public:
    // IRHI interface
    virtual void Init() final override;
    virtual void Shutdown() final override;
    virtual void Resize() final override;
    virtual void Render() final override;

    PVulkanInstance* GetInstance() const;
    PVulkanDevice* GetDevice() const;
    PVulkanMemory* GetMemory() const;
    PVulkanSceneRenderer* GetSceneRenderer() const;

    std::vector<const char*> ValidationLayerExtensions = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> InstanceExtensions = { "VK_EXT_debug_utils" };
    std::vector<const char*> PhysicalDeviceExtensions = { "VK_KHR_swapchain" };

private:
    PVulkanInstance* Instance;
    PVulkanDevice* Device;
    PVulkanMemory* Memory;
    PVulkanSceneRenderer* SceneRenderer;
};
