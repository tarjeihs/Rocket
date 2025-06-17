#pragma once

#include "VulkanDependency.h"
#include "Renderer/Common/RHIFrame.h"
#include "Renderer/Common/RHIViewport.h"

class IRHIFence;
class IRHISemaphore;

struct CVulkanBackbuffer
{
    VkImage        image          = VK_NULL_HANDLE;
    VkImageView    view           = VK_NULL_HANDLE;
};

struct CVulkanFrame : IRHIFrame
{
    FVulkanSemaphore ImageAvailableSemaphore;        // signalled by acquire
    FVulkanSemaphore RenderFinishedSemaphore;        // signalled by queue
    FVulkanFence InFlightFence;                      // waited by CPU
    IRHICommandList* CommandList;                  // primary CB
    uint32_t ImageIndex = 0;
};

class CVulkanViewport : public IRHIViewport
{
public:
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void Present() override;
    virtual void Resize() override;

    void Initialize();
    void Shutdown();

    void CreateSwapchain();
    void DestroySwapchain();

    VkSwapchainKHR Swapchain;
    VkSurfaceKHR Surface;
    VkExtent2D Extent;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;

    uint32_t FrameIndex = 0;

    std::vector<CVulkanBackbuffer> Backbuffer;
    std::vector<CVulkanFrame> Frame;
};