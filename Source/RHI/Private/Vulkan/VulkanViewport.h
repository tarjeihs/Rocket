#pragma once

class FVulkanDevice;

class FVulkanViewport
{
private:
    FVulkanDevice& Device;

public:
    FVulkanViewport(FVulkanDevice& InDevice);
    ~FVulkanViewport();

    bool Acquire();
    void Present();

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetExtent() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

    inline VkSemaphore GetImageAcquiredSemaphore() const;
    inline VkSemaphore GetRenderFinishedSemaphore() const;
    inline VkImage GetImage() const;
    inline VkImageView GetImageView() const;

private:
    void CreateSwapchain();
    void DestroySwapchain();
    
private:
    VkSurfaceKHR Surface;
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;

    std::vector<VkImage> Images;
    std::vector<VkImageView> Views;
    std::vector<VkSemaphore> ImageAcquiredSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;

    uint32_t CurrentImageIndex;
    uint32_t SemaphoreIndex ;
    uint32_t NumAcquireCalls;
    uint32_t NumPresentCalls;
};

inline VkSurfaceKHR FVulkanViewport::GetVkSurface() const
{
    return Surface;
}

inline VkSwapchainKHR FVulkanViewport::GetVkSwapchain() const
{
    return Swapchain;
}

inline VkExtent2D FVulkanViewport::GetExtent() const
{
    return Extent;
}

inline VkPresentModeKHR FVulkanViewport::GetPresentMode() const
{
    return PresentMode;
}

inline VkSurfaceFormatKHR FVulkanViewport::GetSurfaceFormat() const
{
    return SurfaceFormat;
}

inline VkSemaphore FVulkanViewport::GetImageAcquiredSemaphore() const
{
    return ImageAcquiredSemaphores[SemaphoreIndex];
}

inline VkSemaphore FVulkanViewport::GetRenderFinishedSemaphore() const
{
    return RenderFinishedSemaphores[CurrentImageIndex];
}

inline VkImage FVulkanViewport::GetImage() const
{
    return Images[CurrentImageIndex];
}

inline VkImageView FVulkanViewport::GetImageView() const
{
    return Views[CurrentImageIndex];
}
