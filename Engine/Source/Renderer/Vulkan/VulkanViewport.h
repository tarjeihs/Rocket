#pragma once

#include "Renderer/Common/RHIViewport.h"

class CVulkanDevice;

class CVulkanViewport : public IRHIViewport
{
public:
    virtual void Swap(IRHICommandList& CmdList) override;
    virtual void Present(IRHICommandList& CmdList) override;

    void CreateViewport(CVulkanDevice* Device);
    void CreateSwapchain(CVulkanDevice* Device);

    void FreeViewport(CVulkanDevice* Device);
    void FreeSwapchain(CVulkanDevice* Device);

    void GetSurfaceFormats(VkPhysicalDevice InPhysicalDevice, std::vector<VkSurfaceFormatKHR>& OutSurfaceFormats) const;
    void GetPresentModes(VkPhysicalDevice InPhysicalDevice, std::vector<VkPresentModeKHR>& OutPresentMode) const;

    VkSwapchainKHR SwapchainKHR;
    VkSurfaceKHR Surface;

    std::vector<VkImage> Backbuffer;
    std::vector<VkImageView> BackbufferView;
};