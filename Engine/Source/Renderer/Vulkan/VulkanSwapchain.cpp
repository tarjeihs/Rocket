#include "EnginePCH.h"
#include "VulkanSwapchain.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanInstance.h"

// Prioritize VK_PRESENT_MODE_IMMEDIATE_KHR to disable V-Sync
static constexpr VkPresentModeKHR PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

namespace Utils
{
	static VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
	{
		for (const auto& Format : Formats)
		{
			if (Format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && Format.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
			{
				return Format;
			}

			// Prefer SRGB if available (results in more accurate perceived colors and is the golden standard).
			if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return Format;
			}
		}
		return Formats[0];
	}

	static VkPresentModeKHR SelectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes, VkPresentModeKHR DesiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR)
	{
		for (const auto& PresentMode : PresentModes)
		{
			if (PresentMode == DesiredPresentMode)
			{
				return PresentMode;
			}
		}
		// Guaranteed to be avilable (standard V-Sync)
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D SelectSwapchainSurfaceExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
	{
		if (Capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			int32_t Width, Height;
			glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);

			VkExtent2D Extent = { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };

			Extent.width = Math::Clamp(Extent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
			Extent.height = Math::Clamp(Extent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

			return Extent;
		}
		else
		{
			return Capabilities.currentExtent;
		}
	}
}

void PVulkanSwapchain::Init()
{
	Info.SwapchainSurfaceFormat = Utils::SelectSwapchainSurfaceFormat(GetRHI()->GetDevice()->GetSurfaceFormats());
	Info.SwapchainPresentMode = Utils::SelectSwapchainPresentMode(GetRHI()->GetDevice()->GetPresentModes(), Info.SwapchainPresentMode);
	Info.SwapchainImageExtent = Utils::SelectSwapchainSurfaceExtent(GetRHI()->GetDevice()->GetSurfaceCapabilities());

	// Number of images to use in the swapchain
	uint32_t ImageCount = GetRHI()->GetDevice()->GetSurfaceCapabilities().minImageCount + 1;
	if (GetRHI()->GetDevice()->GetSurfaceCapabilities().maxImageCount > 0 && ImageCount > GetRHI()->GetDevice()->GetSurfaceCapabilities().maxImageCount)
	{
		ImageCount = GetRHI()->GetDevice()->GetSurfaceCapabilities().maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = GetRHI()->GetInstance()->GetVkSurfaceKHR();
	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = Info.SwapchainSurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = Info.SwapchainSurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = Info.SwapchainImageExtent;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	SwapchainCreateInfo.preTransform = GetRHI()->GetDevice()->GetSurfaceCapabilities().currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.presentMode = Info.SwapchainPresentMode;
	SwapchainCreateInfo.clipped = VK_TRUE;
	SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value() != GetRHI()->GetDevice()->GetPresentFamilyIndex().value())
	{
		std::vector<uint32_t> QueueFamilyIndices = { GetRHI()->GetDevice()->GetGraphicsFamilyIndex().value(), GetRHI()->GetDevice()->GetPresentFamilyIndex().value() };

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

	VkResult Result = vkCreateSwapchainKHR(GetRHI()->GetDevice()->GetVkDevice(), &SwapchainCreateInfo, nullptr, &Info.SwapchainKHR);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

	// Obtain the array of presentable images associated with a swapchain
	TArray<VkImage> QuerySwapchainImages;
	vkGetSwapchainImagesKHR(GetRHI()->GetDevice()->GetVkDevice(), Info.SwapchainKHR, &ImageCount, nullptr);
	QuerySwapchainImages.Resize(ImageCount);
	Info.Backbuffer.Resize(ImageCount);
	vkGetSwapchainImagesKHR(GetRHI()->GetDevice()->GetVkDevice(), Info.SwapchainKHR, &ImageCount, QuerySwapchainImages.GetData());
	
	for (size_t Index = 0; Index < ImageCount; ++Index)
	{
		FVkImage* Image = new FVkImage();
		Image->Info.ImageHandle = QuerySwapchainImages[Index];
		Image->Info.Format = Info.SwapchainSurfaceFormat.format;
		Image->Info.Extent = Info.SwapchainImageExtent;
		
		VkImageViewCreateInfo ImageViewCreateInfo = {};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.image = Image->Info.ImageHandle;
		ImageViewCreateInfo.format = Image->Info.Format;
		ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		Result = vkCreateImageView(GetRHI()->GetDevice()->GetVkDevice(), &ImageViewCreateInfo, nullptr, &Image->Info.ImageViewHandle);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");

		Info.Backbuffer[Index] = Image;
	}
}

void PVulkanSwapchain::Shutdown()
{
	while (Info.Backbuffer.GetSize())
	{
		FVkImage* Image = Info.Backbuffer[0];
		vkDestroyImageView(GetRHI()->GetDevice()->GetVkDevice(), Image->Info.ImageViewHandle, VK_NULL_HANDLE);

		delete Image;
		Info.Backbuffer.RemoveAt(0);
	}
	
	// Vulkan does internal destruction of swapchain images
	vkDestroySwapchainKHR(GetRHI()->GetDevice()->GetVkDevice(), Info.SwapchainKHR, nullptr);
}