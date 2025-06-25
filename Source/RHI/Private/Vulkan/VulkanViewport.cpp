#include "RocketPCH.h"
#include "RHI/Private/Vulkan/VulkanViewport.h"

#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"
#include "RHI/Private/Vulkan/VulkanDevice.h"
#include <cstdio>

FVulkanViewport::FVulkanViewport(FVulkanDevice& InDevice)
    : Device(InDevice),
    Swapchain(nullptr),
    Surface(nullptr),
    CurrentImageIndex(UINT32_MAX),
    SemaphoreIndex(0),
    NumAcquireCalls(0),
    NumPresentCalls(0)
{
    VkResult Result = glfwCreateWindowSurface(GetVulkanRHIMinimal()->RHIGetVkInstance(), static_cast<GLFWwindow *>(GetWindow()->GetNativeWindow()), nullptr, &Surface);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan surface.");

    uint32_t SurfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
    if (SurfaceFormatCount)
    {
        SurfaceFormats.resize(SurfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, SurfaceFormats.data());
    }

    for (const auto& Element : SurfaceFormats)
    {
        if (Element.format == VK_FORMAT_B8G8R8A8_SRGB && Element.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = Element;
        }
    }

    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, nullptr);

    std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
    if (PresentModeCount)
    {
        PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, PresentModes.data());
    }

    for (const auto& Element : PresentModes)
    {
        if (Element == VK_PRESENT_MODE_FIFO_KHR)
        {
            PresentMode = Element;
        }
    }

    CreateSwapchain();
}

FVulkanViewport::~FVulkanViewport()
{
    DestroySwapchain();

    vkDestroySurfaceKHR(GetVulkanRHIMinimal()->RHIGetVkInstance(), Surface, nullptr);
}

void FVulkanViewport::CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceCapabilities);

    int32_t Width, Height;
    glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);

    Extent = { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };
    Extent.width = std::clamp(Extent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
    Extent.height = std::clamp(Extent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

    // Surface size is fixed (eg. fullscreen)
    if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = SurfaceCapabilities.currentExtent;
    }

    uint32_t ImageCount = SurfaceCapabilities.minImageCount;
    if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount)
    {
        ImageCount = SurfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
    SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCreateInfo.surface = Surface;
    SwapchainCreateInfo.minImageCount = ImageCount;
    SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapchainCreateInfo.imageExtent = Extent;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    SwapchainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;
    SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    SwapchainCreateInfo.clipped = VK_TRUE;

    if (GetVulkanRHIMinimal()->RHIGetGraphicsQueueFamilyIndex() != GetVulkanRHIMinimal()->RHIGetPresentQueueFamilyIndex())
    {
        std::vector<uint32_t> QueueFamilyIndices = { GetVulkanRHIMinimal()->RHIGetGraphicsQueueFamilyIndex(), GetVulkanRHIMinimal()->RHIGetPresentQueueFamilyIndex() };

        // Images can be used across multiple queue families without explicit ownership transfers.
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(QueueFamilyIndices.size());
        SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices.data();
    }
    else
    {
        // An image is owned by one queue family at a time and ownership must be explicitly
        // transferred before using it in another queue family. This option offers the best performance.
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult Result = vkCreateSwapchainKHR(Device.GetVkDevice(), &SwapchainCreateInfo, nullptr, &Swapchain);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

    RK_LOG_DEBUG("Swapchain: {} images, {} frames-in-flight", ImageCount, ImageCount - 1);

    vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    Views.resize(ImageCount);
    vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, Images.data());

    for (uint32_t Index = 0; Index < ImageCount; ++Index)
    {
        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewCreateInfo.image = Images[Index];
        ImageViewCreateInfo.format = SurfaceFormat.format;
        ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ImageViewCreateInfo.subresourceRange.levelCount = 1;
        ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewCreateInfo.subresourceRange.layerCount = 1;
        ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        VkResult Result = vkCreateImageView(Device.GetVkDevice(), &ImageViewCreateInfo, nullptr, &Views[Index]);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
    }
    
    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    ImageAcquiredSemaphores.resize(ImageCount - 1);
    RenderFinishedSemaphores.resize(ImageCount);

    for (uint32_t Index = 0; Index < ImageAcquiredSemaphores.size(); ++Index)
    {
        vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &ImageAcquiredSemaphores[Index]);
    }

    for (uint32_t Index = 0; Index < RenderFinishedSemaphores.size(); ++Index)
    {
        vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphores[Index]);
    }
}

bool FVulkanViewport::Acquire()
{
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphores.size();

    VkResult Result = vkAcquireNextImageKHR(GetVulkanRHIMinimal()->RHIGetVkDevice(), Swapchain, UINT64_MAX, ImageAcquiredSemaphores[SemaphoreIndex], VK_NULL_HANDLE, &CurrentImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        DestroySwapchain();
        CreateSwapchain();
        return false;
    }

    ++NumAcquireCalls;
    return true;
}

void FVulkanViewport::Present()
{
    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.pNext = VK_NULL_HANDLE;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentImageIndex];
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pImageIndices = &CurrentImageIndex;

    vkQueuePresentKHR(GetVulkanRHIMinimal()->RHIGetPresentVkQueue(), &PresentInfo);

    ++NumPresentCalls;
}

void FVulkanViewport::DestroySwapchain()
{
    Device.WaitUntilIdle();

    for (auto& Semaphore : ImageAcquiredSemaphores)
    {
        vkDestroySemaphore(Device.GetVkDevice(), Semaphore, nullptr);
    }

    for (auto& Semaphore : RenderFinishedSemaphores)
    {
        vkDestroySemaphore(Device.GetVkDevice(), Semaphore, nullptr);
    }

    for (auto& view : Views)
    {
        vkDestroyImageView(Device.GetVkDevice(), view, nullptr);
    }

    vkDestroySwapchainKHR(Device.GetVkDevice(), Swapchain, nullptr);

    Views.clear();
    Images.clear();
    ImageAcquiredSemaphores.clear();
    RenderFinishedSemaphores.clear();

    CurrentImageIndex = UINT32_MAX;
    SemaphoreIndex = 0;
    NumAcquireCalls = 0;
    NumPresentCalls = 0;
}
