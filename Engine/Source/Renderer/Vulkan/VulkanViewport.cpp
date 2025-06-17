#include "EnginePCH.h"
#include "VulkanViewport.h"

#include "VulkanCommandList.h"
#include "VulkanDevice.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"
#include "Renderer/Common/RHICommandList.h"
#include "Renderer/Common/RHIDependency.h"
#include "Renderer/Common/RHIPayload.h"

void CVulkanViewport::BeginFrame()
{
    VkResult Result = VK_SUCCESS;
    Result = vkWaitForFences((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), 1, &Frame[FrameIndex].InFlightFence.Handle, VK_TRUE, UINT64_MAX);
    Result = vkAcquireNextImageKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Swapchain, UINT64_MAX, Frame[FrameIndex].ImageAvailableSemaphore.Handle, VK_NULL_HANDLE, &Frame[FrameIndex].ImageIndex);
    // Safely reset fence after successful image acquire.
    Result = vkResetFences((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), 1, &Frame[FrameIndex].InFlightFence.Handle);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        Resize();
        BeginFrame();
        return;
    }

    Frame[FrameIndex].CommandList->Reset();
    Frame[FrameIndex].CommandList->Begin();
    Frame[FrameIndex].CommandList->Execute();
}

void CVulkanViewport::EndFrame()
{
    SetImageLayout(Backbuffer[Frame[FrameIndex].ImageIndex].image, (VkCommandBuffer)Frame[FrameIndex].CommandList->GetNativeHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    Frame[FrameIndex].CommandList->End();

    FRHIPayload Payload = {
        Frame[FrameIndex].CommandList,
        &Frame[FrameIndex].ImageAvailableSemaphore,
        &Frame[FrameIndex].RenderFinishedSemaphore,
        &Frame[FrameIndex].InFlightFence
    };

    GetRHI()->GetDevice()->GetGraphicsQueue()->Submit(&Payload);
}

void CVulkanViewport::Present()
{
    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.pNext = VK_NULL_HANDLE;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pWaitSemaphores = &Frame[FrameIndex].RenderFinishedSemaphore.Handle;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pImageIndices = &Frame[FrameIndex].ImageIndex;

    vkQueuePresentKHR((VkQueue)GetRHI()->GetDevice()->GetGraphicsQueue()->GetNativeHandle(), &PresentInfo);

    FrameIndex = (FrameIndex + 1) % Frame.size();
}

void CVulkanViewport::Resize()
{
    Frame[FrameIndex].RenderFinishedSemaphore.Invalidate();
    Frame[FrameIndex].ImageAvailableSemaphore.Invalidate();

    DestroySwapchain();
    CreateSwapchain();
}

void CVulkanViewport::Initialize()
{
    VkResult Result = glfwCreateWindowSurface((VkInstance)GetRHI()->GetDevice()->GetNativeInstance(), static_cast<GLFWwindow *>(GetWindow()->GetNativeWindow()), nullptr, &Surface);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan surface.");

    CreateSwapchain();

    for (uint32 Index = 0; Index < Frame.size(); Index++)
    {
        Frame[Index].InFlightFence.Invalidate();
        Frame[Index].RenderFinishedSemaphore.Invalidate();
        Frame[Index].ImageAvailableSemaphore.Invalidate();

        CVulkanCommandList* CommandList = new CVulkanCommandList();
        CommandList->Initialize();
        Frame[Index].CommandList = CommandList;
    }
}

void CVulkanViewport::Shutdown()
{
    DestroySwapchain();

    for (CVulkanFrame& Element : Frame)
    {
        vkDestroyFence((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Element.InFlightFence.Handle, nullptr);
        vkDestroySemaphore((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Element.ImageAvailableSemaphore.Handle, nullptr);
        vkDestroySemaphore((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Element.RenderFinishedSemaphore.Handle, nullptr);

        Element.CommandList->Shutdown();
        delete Element.CommandList;
    }

    Frame.clear();

    // Vulkan handles internal destruction of swapchain images
    vkDestroySwapchainKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Swapchain, nullptr);
	vkDestroySurfaceKHR((VkInstance)GetRHI()->GetDevice()->GetNativeInstance(), Surface, nullptr);

    Swapchain = VK_NULL_HANDLE;
    Surface = VK_NULL_HANDLE;
}

void CVulkanViewport::CreateSwapchain()
{
    uint32_t SurfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR((VkPhysicalDevice)GetRHI()->GetDevice()->GetNativeDevice(), Surface, &SurfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
    if (SurfaceFormatCount)
    {
        SurfaceFormats.resize(SurfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR((VkPhysicalDevice)GetRHI()->GetDevice()->GetNativeDevice(), Surface, &SurfaceFormatCount, SurfaceFormats.data());
    }

    for (const auto& Element : SurfaceFormats)
    {
        if (Element.format == VK_FORMAT_B8G8R8A8_SRGB && Element.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = Element;
        }
    }

    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR((VkPhysicalDevice)GetRHI()->GetDevice()->GetNativeDevice(), Surface, &PresentModeCount, nullptr);

    std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
    if (PresentModeCount)
    {
        PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR((VkPhysicalDevice)GetRHI()->GetDevice()->GetNativeDevice(), Surface, &PresentModeCount, PresentModes.data());
    }

    for (const auto& Element : PresentModes)
    {
        if (Element == VK_PRESENT_MODE_FIFO_KHR)
        {
            PresentMode = Element;
        }
    }

	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR((VkPhysicalDevice)GetRHI()->GetDevice()->GetNativeDevice(), Surface, &SurfaceCapabilities);

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

    // Number of images to use in the swapchain
    uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;
    if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount)
    {
        ImageCount = SurfaceCapabilities.maxImageCount;
    }
    uint32_t FrameCount = std::min(2u, ImageCount);

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
    SwapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.oldSwapchain = nullptr; // TODO: Passing the live swapchain handle would result in greater performance.

    if (GetRHI()->GetDevice()->GetPresentQueue() != GetRHI()->GetDevice()->GetGraphicsQueue())
    {
        std::vector<uint32_t> QueueFamilyIndices = { GetRHI()->GetDevice()->GetPresentQueue()->GetCapabilities().Engine, GetRHI()->GetDevice()->GetGraphicsQueue()->GetCapabilities().Engine };

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

    VkResult Result = vkCreateSwapchainKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &SwapchainCreateInfo, nullptr, &Swapchain);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

    // Obtain an array of presentable images associated with the swapchain
    std::vector<VkImage> SwapchainImages;

    vkGetSwapchainImagesKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Swapchain, &ImageCount, nullptr);
    SwapchainImages.resize(ImageCount);
    Backbuffer.resize(ImageCount);
    Frame.resize(FrameCount);
    vkGetSwapchainImagesKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Swapchain, &ImageCount, SwapchainImages.data());

    for (uint32 Index = 0; Index < ImageCount; ++Index)
    {
        Backbuffer[Index].image = SwapchainImages[Index];

        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewCreateInfo.image = Backbuffer[Index].image;
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

        VkResult Result = vkCreateImageView((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &ImageViewCreateInfo, nullptr, &Backbuffer[Index].view);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
    }
}

void CVulkanViewport::DestroySwapchain()
{
    for (CVulkanBackbuffer& Element : Backbuffer)
    {
        vkDestroyImageView((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Element.view, nullptr);
    }
    Backbuffer.clear();
    vkDestroySwapchainKHR((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Swapchain, nullptr);
}
