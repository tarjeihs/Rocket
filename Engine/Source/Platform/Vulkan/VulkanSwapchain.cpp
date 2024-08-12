#include "EnginePCH.h"
#include "VulkanSwapchain.h"

#include <GLFW/glfw3.h>

#include "Core/Assert.h"
#include "Core/Window.h"
#include "Math/Math.h"
#include "Platform/Vulkan/VulkanUtility.h"

VkSurfaceFormatKHR PVulkanSwapchain::SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
{
	for (const auto& Format : Formats)
	{
		// Prefer SRGB if available (results in more accurate perceived colors and is the golden standard).
		if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return Format;
		}
	}
	return Formats[0];
}

VkPresentModeKHR PVulkanSwapchain::SelectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
	for (const auto& PresentMode : PresentModes)
	{
		// Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. 
		// This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync
		if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return PresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D PVulkanSwapchain::SelectSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
	if (Capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		int32_t Width, Height;
		glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);

		VkExtent2D Extent = {
			static_cast<uint32_t>(Width),
			static_cast<uint32_t>(Height)
		};

		Extent.width = Math::Clamp(Extent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
		Extent.height = Math::Clamp(Extent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

		return Extent;
	}
	else
	{
		return Capabilities.currentExtent;
	}
}

void PVulkanSwapchain::CreateSwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator)
{
	SSwapchainSupportDetails SwapchainSupportDetails = Vulkan::Utility::RequestSwapchainSupportDetails(SurfaceInterface, PhysicalDevice);

	VkSurfaceFormatKHR SurfaceFormat = SelectSwapchainSurfaceFormat(SwapchainSupportDetails.Formats);
	VkPresentModeKHR PresentMode = SelectSwapchainPresentMode(SwapchainSupportDetails.PresentMode);
	VkExtent2D Extent = SelectSwapExtent(SwapchainSupportDetails.Capabilities);

	// Number of images to use in our swapchain
	uint32_t ImageCount = SwapchainSupportDetails.Capabilities.minImageCount + 1;
	if (SwapchainSupportDetails.Capabilities.maxImageCount > 0 && ImageCount > SwapchainSupportDetails.Capabilities.maxImageCount)
	{
		ImageCount = SwapchainSupportDetails.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = SurfaceInterface;
	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = SurfaceFormat.format;
	CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	CreateInfo.imageExtent = Extent;
	CreateInfo.imageArrayLayers = 1;
	//CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	SQueueFamilyIndices Indices = Vulkan::Utility::RequestQueueFamilies(SurfaceInterface, PhysicalDevice);
	uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

	if (Indices.GraphicsFamily != Indices.PresentFamily)
	{
		// Images can be used across multiple queue families without explicit ownership transfers.
		CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = 2;
		CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else
	{
		// An image is owned by one queue family at a time and ownership must be explicitly 
		// transferred before using it in another queue family. This option offers the best performance.
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	CreateInfo.preTransform = SwapchainSupportDetails.Capabilities.currentTransform;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.presentMode = PresentMode;
	CreateInfo.clipped = VK_TRUE;
	CreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult Result = vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, nullptr, &SwapchainKHR);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

	vkGetSwapchainImagesKHR(LogicalDevice, SwapchainKHR, &ImageCount, nullptr);
	SwapchainImages.resize(ImageCount);
	vkGetSwapchainImagesKHR(LogicalDevice, SwapchainKHR, &ImageCount, SwapchainImages.data());

	SwapchainImageFormat = SurfaceFormat.format;
	SwapchainExtent = Extent;

	// To use any of the images in the swapchain, a VkImageView object is required (a readonly view into the image).
	SwapchainImageViews.resize(ImageCount);
	for (size_t Idx = 0; Idx < ImageCount; Idx++)
	{
		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.image = SwapchainImages[Idx];
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Specifies how an image should be interpreted (eg. 1D textures, 2D textures, 3D textures and cube maps).
		ImageViewCreateInfo.format = SwapchainImageFormat;

		// Default color channel mapping
		ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Defines image purpose and what part of the image should be accessed. 
		// Image is currently set to be used as color targets without any mipmapping levels or multiple layers.
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VkResult Result = vkCreateImageView(LogicalDevice, &ImageViewCreateInfo, nullptr, &SwapchainImageViews[Idx]);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
	}
	
	RenderTargetExtent = SwapchainExtent;
	VkExtent3D DrawImageExtent = { RenderTargetExtent.width, RenderTargetExtent.height, 1 };

	RenderTarget.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT; // Or another supported format
	RenderTarget.imageExtent = DrawImageExtent;

	VkImageUsageFlags DrawImageUsageFlags{};
	DrawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	DrawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	DrawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	DrawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo DrawImageCreateInfo{};
	DrawImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	DrawImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	DrawImageCreateInfo.format = RenderTarget.imageFormat;
	DrawImageCreateInfo.extent = RenderTarget.imageExtent;
	DrawImageCreateInfo.mipLevels = 1;
	DrawImageCreateInfo.arrayLayers = 1;
	DrawImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	DrawImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	DrawImageCreateInfo.usage = DrawImageUsageFlags;

	// Allocate and create the image
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vmaCreateImage(Allocator, &DrawImageCreateInfo, &rimg_allocinfo, &RenderTarget.image, &RenderTarget.allocation, nullptr);

	// Create image view
	VkImageViewCreateInfo DrawImageViewCreateInfo{};
	DrawImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	DrawImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	DrawImageViewCreateInfo.image = RenderTarget.image;
	DrawImageViewCreateInfo.format = RenderTarget.imageFormat;
	DrawImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	DrawImageViewCreateInfo.subresourceRange.levelCount = 1;
	DrawImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	DrawImageViewCreateInfo.subresourceRange.layerCount = 1;
	DrawImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	Result = vkCreateImageView(LogicalDevice, &DrawImageViewCreateInfo, nullptr, &RenderTarget.imageView);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
}

void PVulkanSwapchain::RegenerateSwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator)
{
	vkDeviceWaitIdle(LogicalDevice);

	while (GetWindow()->IsMinimized())
	{
		// Rough handling of window minimization.
		glfwWaitEvents();
	}

	DestroySwapchain(SurfaceInterface, PhysicalDevice, LogicalDevice, Allocator);

	CreateSwapchain(SurfaceInterface, PhysicalDevice, LogicalDevice, Allocator);
}

void PVulkanSwapchain::DestroySwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator)
{
	for (size_t i = 0; i < SwapchainImageViews.size(); i++)
	{
		vkDestroyImageView(LogicalDevice, SwapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(LogicalDevice, SwapchainKHR, nullptr);
}

