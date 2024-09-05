#include "VulkanSwapchain.h"

#include <optional>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "Core/Assert.h"
#include "Core/Window.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanInstance.h"
#include "Math/Math.h"
// Prioritize VK_PRESENT_MODE_IMMEDIATE_KHR to disable V-Sync
static constexpr VkPresentModeKHR PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

namespace Utils
{
	static VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
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
	const PVulkanRHI* RHI = GetRHI<PVulkanRHI>();

	SurfaceFormat = Utils::SelectSwapchainSurfaceFormat(RHI->GetDevice()->GetSurfaceFormats());
	PresentMode = Utils::SelectSwapchainPresentMode(RHI->GetDevice()->GetPresentModes(), PresentMode);
	ImageExtent = Utils::SelectSwapchainSurfaceExtent(RHI->GetDevice()->GetSurfaceCapabilities());

	// Number of images to use in the swapchain
	uint32_t ImageCount = RHI->GetDevice()->GetSurfaceCapabilities().minImageCount + 1;
	if (RHI->GetDevice()->GetSurfaceCapabilities().maxImageCount > 0 && ImageCount > RHI->GetDevice()->GetSurfaceCapabilities().maxImageCount)
	{
		ImageCount = RHI->GetDevice()->GetSurfaceCapabilities().maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = RHI->GetInstance()->GetVkSurfaceKHR();
	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = ImageExtent;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	SwapchainCreateInfo.preTransform = RHI->GetDevice()->GetSurfaceCapabilities().currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.presentMode = PresentMode;
	SwapchainCreateInfo.clipped = VK_TRUE;
	SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (RHI->GetDevice()->GetGraphicsFamilyIndex().value() != RHI->GetDevice()->GetPresentFamilyIndex().value())
	{
		std::vector<uint32_t> QueueFamilyIndices = { RHI->GetDevice()->GetGraphicsFamilyIndex().value(), RHI->GetDevice()->GetPresentFamilyIndex().value() };

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

	VkResult Result = vkCreateSwapchainKHR(RHI->GetDevice()->GetVkDevice(), &SwapchainCreateInfo, nullptr, &SwapchainKHR);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

	// Obtain the array of presentable images associated with a swapchain
	std::vector<VkImage> QuerySwapchainImages;
	vkGetSwapchainImagesKHR(RHI->GetDevice()->GetVkDevice(), SwapchainKHR, &ImageCount, nullptr);
	QuerySwapchainImages.resize(ImageCount);
	SwapchainImages.resize(ImageCount);
	vkGetSwapchainImagesKHR(RHI->GetDevice()->GetVkDevice(), SwapchainKHR, &ImageCount, QuerySwapchainImages.data());
	
	for (size_t Index = 0; Index < ImageCount; ++Index)
	{
		PVulkanImage* Image = new PVulkanImage();
		Image->Init(ImageExtent, SurfaceFormat.format);
		Image->ApplyImage(QuerySwapchainImages[Index]);
		Image->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		SwapchainImages[Index] = Image;
	}
}

void PVulkanSwapchain::Shutdown()
{
	const PVulkanRHI* RHI = GetRHI<PVulkanRHI>();

	while (SwapchainImages.size())
	{
		PVulkanImage* Image = SwapchainImages[0];
		Image->DestroyImageView();

		delete Image;
		SwapchainImages.erase(SwapchainImages.begin());
	}
	
	// Vulkan does internal destruction of swapchain images
	vkDestroySwapchainKHR(RHI->GetDevice()->GetVkDevice(), SwapchainKHR, nullptr);
}

VkSwapchainKHR PVulkanSwapchain::GetVkSwapchain() const
{
	return SwapchainKHR;
}

VkSurfaceFormatKHR PVulkanSwapchain::GetSurfaceFormat() const
{
	return SurfaceFormat;
}

VkExtent2D PVulkanSwapchain::GetVkExtent() const
{
	return ImageExtent;
}

const std::vector<PVulkanImage*>& PVulkanSwapchain::GetSwapchainImages() const
{
	return SwapchainImages;
}