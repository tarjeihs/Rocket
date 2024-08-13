#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vk_mem_alloc.h>
#include <optional>

#include "Core/Assert.h"
#include "Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanDescriptor.h"

class SValidationLayer
{
public:
	SValidationLayer(const char* InName)
		: Name(InName)
	{
		RK_ENGINE_ASSERT(IsSupported(), "This Validation Layer is not supported.");
	}

	bool IsSupported();

	const char* Name;
	bool bIsValidFlag = true;
};

constexpr unsigned int FRAME_OVERLAP = 2;

struct SFrameData
{
	VkCommandPool CommandPool;
	VkCommandBuffer CommandBuffer;

	VkSemaphore SwapchainSemaphore;
	VkSemaphore RenderSemaphore;

	VkFence RenderFence;
};

struct SVulkanInstance
{
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR SurfaceInterface;
	VkInstance Handle;

	std::vector<SValidationLayer> ValidationLayers;
	std::vector<const char*> Extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

struct SVulkanDevice
{
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
	
	// Supports graphics operations such as drawing, compute, and transfer operations. Used to submit rendering commands (like drawing triangles).
	VkQueue GraphicsQueue;

	// Specifically used to submit images to be presented to a surface (GLFW window). It may or may not be the same as the graphics queue.
	VkQueue PresentQueue;

	uint32_t GraphicsFamilyIndex;
};

class PVulkanSwapchain;
class PVulkanMemory;

class PVulkanRenderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void DestroyRenderer() override;
	virtual void Draw() override;

	inline PVulkanSwapchain* GetSwapchain() const;
	inline const SVulkanInstance& GetInstance() const;
	inline const SVulkanDevice& GetDevice() const;
	inline VmaAllocator GetAllocator() const;

protected:
	void CreateInstance();
	void CreateSurfaceInterface();

	void SetupDebugMessenger();
	void DestroyDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
	VkDebugUtilsMessengerEXT GetDebugMessenger() const;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
		VkDebugUtilsMessageTypeFlagsEXT Type,
		const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
		void* UserData);

public:
	void CreatePhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();
	void CreateSynchronizationObjects();
	void CreateMemoryAllocator();
	void CreateDescriptorSet();
	void CreatePipeline();
	void CreateBackgroundPipeline();

	void InitImGui();
	void DrawImGui(VkCommandBuffer CommandBuffer, VkClearValue* ClearValue, VkImageView ImageView);

public:
	void WaitUntilIdle();

protected:
	void ClearBackground(VkCommandBuffer CommandBuffer);

private:
	PVulkanSwapchain* Swapchain;

	SVulkanInstance Instance;
	SVulkanDevice Device;

	SFrameData Frames[FRAME_OVERLAP];
	size_t CurrentFrameIndex = 0;

	VmaAllocator Allocator;
	SDescriptorAllocator DescriptorAllocator;

	VkDescriptorPool ImGuiDescriptorPool;

	VkDescriptorSet RenderTargetDescriptorSet;
	VkDescriptorSetLayout RenderTargetDescriptorSetLayout;

	// Temp
	VkPipeline _gradientPipeline;
	VkPipelineLayout _gradientPipelineLayout;
};

inline PVulkanSwapchain* PVulkanRenderer::GetSwapchain() const
{
	return Swapchain;
}

inline const SVulkanInstance& PVulkanRenderer::GetInstance() const
{
	return Instance;
}

inline const SVulkanDevice& PVulkanRenderer::GetDevice() const
{
	return Device;
}

inline VmaAllocator PVulkanRenderer::GetAllocator() const
{
	return Allocator;
}
