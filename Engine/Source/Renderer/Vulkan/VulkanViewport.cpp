#include "EnginePCH.h"
#include "VulkanViewport.h"

#include "VulkanDevice.h"
#include "Renderer/Common/RHICommandList.h"

void CVulkanViewport::Swap(IRHICommandList& CmdList)
{
    CmdList.Enqueue([=]()
    {

    });
}

void CVulkanViewport::Present(IRHICommandList& CmdList)
{
    CmdList.Enqueue([=]()
    {

    });
}

void CVulkanViewport::CreateViewport(CVulkanDevice *Device)
{
    VkResult Result = glfwCreateWindowSurface(Device->Instance, static_cast<GLFWwindow *>(GetWindow()->GetNativeWindow()), nullptr, &Surface);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan surface.");
}

void CVulkanViewport::CreateSwapchain(CVulkanDevice *Device)
{
    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    GetSurfaceFormats(Device->PhysicalDevice, SurfaceFormats);
    VkSurfaceFormatKHR SurfaceFormat;

    for (const auto& SurfaceFormatItem : SurfaceFormats)
    {
        if (SurfaceFormatItem.format == VK_FORMAT_B8G8R8A8_SRGB && SurfaceFormatItem.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = SurfaceFormatItem;
        }
    }

    std::vector<VkPresentModeKHR> PresentModes;
    GetPresentModes(Device->PhysicalDevice, PresentModes);
    VkPresentModeKHR PresentMode;

    for (const auto& PresentModeItem : PresentModes)
    {
        if (PresentModeItem == VK_PRESENT_MODE_FIFO_KHR)
        {
            PresentMode = PresentModeItem;
        }
    }

    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->PhysicalDevice, Surface, &SurfaceCapabilities);

    int32_t Width, Height;
    glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);

    VkExtent2D Extent = { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };

    Extent.width = std::clamp(Extent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
    Extent.height = std::clamp(Extent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

    // Surface size is fixed (eg. fullscreen)
    if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = SurfaceCapabilities.currentExtent;
    }

    // Number of images to use in the swapchain
    uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;
    if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount)
    {
        ImageCount = SurfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
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
    SwapchainCreateInfo.presentMode = PresentMode;
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    std::optional<uint32> GraphicsQueueIndex;
    Device->GetGraphicsQueueFamily(Device->PhysicalDevice, Surface, GraphicsQueueIndex);

    std::optional<uint32> PresentQueueIndex;
    Device->GetPresentQueueFamily(Device->PhysicalDevice, Surface, PresentQueueIndex);

    if (GraphicsQueueIndex.value() != PresentQueueIndex.value())
    {
        std::vector<uint32_t> QueueFamilyIndices = { GraphicsQueueIndex.value(), PresentQueueIndex.value() };

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

    VkResult Result = vkCreateSwapchainKHR(Device->LogicalDevice, &SwapchainCreateInfo, nullptr, &SwapchainKHR);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

    // Obtain an array of presentable images associated with the swapchain
    vkGetSwapchainImagesKHR(Device->LogicalDevice, SwapchainKHR, &ImageCount, nullptr);
    Backbuffer.resize(ImageCount);
    BackbufferView.resize(ImageCount);
    vkGetSwapchainImagesKHR(Device->LogicalDevice, SwapchainKHR, &ImageCount, Backbuffer.data());

    for (uint32 Index = 0; Index < ImageCount; ++Index)
    {
        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewCreateInfo.image = Backbuffer[Index];
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

        Result = vkCreateImageView(Device->LogicalDevice, &ImageViewCreateInfo, nullptr, &BackbufferView[Index]);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
    }
}

void CVulkanViewport::FreeViewport(CVulkanDevice *Device)
{
    vkDestroySurfaceKHR(Device->Instance, Surface, nullptr);
}

void CVulkanViewport::FreeSwapchain(CVulkanDevice *Device)
{
    for (VkImageView ImageView : BackbufferView)
    {
        vkDestroyImageView(Device->LogicalDevice, ImageView, nullptr);
    }

    Backbuffer.clear();
    BackbufferView.clear();

    // Vulkan handles internal destruction of swapchain images
    vkDestroySwapchainKHR(Device->LogicalDevice, SwapchainKHR, nullptr);
    SwapchainKHR = VK_NULL_HANDLE;
}

void CVulkanViewport::GetSurfaceFormats(VkPhysicalDevice InPhysicalDevice, std::vector<VkSurfaceFormatKHR> &OutSurfaceFormats) const
{
    uint32_t SurfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, Surface, &SurfaceFormatCount, nullptr);

    if (SurfaceFormatCount)
    {
        OutSurfaceFormats.resize(SurfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, Surface, &SurfaceFormatCount, OutSurfaceFormats.data());
    }
}

void CVulkanViewport::GetPresentModes(VkPhysicalDevice InPhysicalDevice, std::vector<VkPresentModeKHR> &OutPresentMode) const
{
    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, Surface, &PresentModeCount, nullptr);

    if (PresentModeCount)
    {
        OutPresentMode.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, Surface, &PresentModeCount, OutPresentMode.data());
    }
}
