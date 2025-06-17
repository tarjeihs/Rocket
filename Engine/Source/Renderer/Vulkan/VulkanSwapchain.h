#pragma once

#include "Renderer/Common/RHISwapchain.h"

class CVulkanSwapchain : public IRHISwapchain
{
public:
    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual void Resize() override;

private:
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;
};