#define VMA_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32

#include "EnginePCH.h"
#include "VulkanRenderer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <set>
#include <vector>

#include "Core/Window.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "VulkanUtility.h"
#include "VulkanTypes.h"
#include "VulkanPipeline.h"

namespace VkUtility
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}

bool SValidationLayer::IsSupported()
{
	uint32_t LayerCount = 0;
	vkEnumerateInstanceLayerProperties(&LayerCount, 0);

	std::vector<VkLayerProperties> LayerProperties(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties.data());

	for (const auto& LayerProperty : LayerProperties)
	{
		if (strcmp(Name, LayerProperty.layerName) == 0)
		{
			return true;
		}
	}
	return false;
}

void PVulkanRenderer::Initialize()
{
	CreateInstance();
	CreateSurfaceInterface();

	CreatePhysicalDevice();
	CreateLogicalDevice();
	CreateGlobalMemoryAllocator();

	Swapchain = new PVulkanSwapchain();
	Swapchain->CreateSwapchain(Instance.SurfaceInterface, Device.PhysicalDevice, Device.LogicalDevice, GlobalMemoryAllocator);

	CreateCommandPool();
	CreateSynchronizationObjects();
	CreateGlobalDescriptorAllocator();

	CreatePipeline();

	InitImGui();
}

void PVulkanRenderer::DestroyRenderer()
{
	vkDeviceWaitIdle(Device.LogicalDevice);

	Swapchain->DestroySwapchain(Instance.SurfaceInterface, Device.PhysicalDevice, Device.LogicalDevice, GlobalMemoryAllocator);

	for (size_t Index = 0; Index < FRAME_OVERLAP; Index++)
	{
		vkDestroySemaphore(Device.LogicalDevice, Frames[Index].RenderSemaphore, nullptr);
		vkDestroySemaphore(Device.LogicalDevice, Frames[Index].SwapchainSemaphore, nullptr);

		vkDestroyFence(Device.LogicalDevice, Frames[Index].RenderFence, nullptr);
		vkDestroyCommandPool(Device.LogicalDevice, Frames[Index].CommandPool, nullptr);
	}
	vkDestroyFence(Device.LogicalDevice, ImmediateFence, nullptr);
	vkDestroyCommandPool(Device.LogicalDevice, ImmediateCommandPool, nullptr);

	GlobalDescriptorAllocator.DeinitializePool(Device.LogicalDevice);
	
	StandardGraphicsPipeline->Free(Device.LogicalDevice);
	StandardGraphicsPipeline->Destroy(Device.LogicalDevice);
	StandardComputePipeline->Free(Device.LogicalDevice);
	StandardComputePipeline->Destroy(Device.LogicalDevice);

	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(Device.LogicalDevice, ImGuiDescriptorPool, nullptr); // TODO: Use the pool allocator instead

	vmaDestroyAllocator(GlobalMemoryAllocator);
	vkDestroyDevice(Device.LogicalDevice, nullptr);
	vkDestroySurfaceKHR(Instance.Handle, Instance.SurfaceInterface, nullptr);
	DestroyDebugMessenger();	
	vkDestroyInstance(Instance.Handle, nullptr);

	delete Swapchain;
}

void PVulkanRenderer::Draw()
{
	SFrameData& CurrentFrame = Frames[CurrentFrameIndex % FRAME_OVERLAP];

	vkWaitForFences(Device.LogicalDevice, 1, &CurrentFrame.RenderFence, true, 1000000000);
	vkResetFences(Device.LogicalDevice, 1, &CurrentFrame.RenderFence);

	// Request image from the swapchain.
	uint32_t SwapchainImageIndex;
	VkResult Result = vkAcquireNextImageKHR(Device.LogicalDevice, Swapchain->SwapchainKHR, 1000000000, CurrentFrame.SwapchainSemaphore, nullptr, &SwapchainImageIndex);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to acquire a new image from the swapchain.");

	VkCommandBufferBeginInfo BeginInfo{};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.pNext = nullptr;
	BeginInfo.pInheritanceInfo = nullptr;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer CommandBuffer = CurrentFrame.CommandBuffer;
	
	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	Result = vkResetCommandBuffer(CommandBuffer, 0);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");

	Result = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to begin command buffer.");

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->DrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	StandardComputePipeline->Draw(CommandBuffer, Swapchain->DrawImage.imageView, Swapchain->DrawImageExtent);

	//transition the draw image and the swapchain image into their correct transfer layouts
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->DrawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	StandardGraphicsPipeline->Draw(CommandBuffer, Swapchain->DrawImage.imageView, Swapchain->DrawImageExtent);

	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->DrawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->SwapchainImages[SwapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	Vulkan::Utility::CopyImageToImage(CommandBuffer, Swapchain->DrawImage.image, Swapchain->SwapchainImages[SwapchainImageIndex], Swapchain->DrawImageExtent, Swapchain->SwapchainExtent);
	
	DrawImGui(CommandBuffer, nullptr, Swapchain->SwapchainImageViews[SwapchainImageIndex]);

	// set swapchain image layout to Present so we can show it on the screen
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->SwapchainImages[SwapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
	Result = vkEndCommandBuffer(CommandBuffer);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to finalize command buffer.");

	// Prepare the submission to the queue. 
	// We want to wait on the SwapchainSemaphore, as that semaphore is signaled when the swapchain is ready
	// We will signal the RenderSemaphore, to signal that rendering has finished

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo{};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.pNext = nullptr;
	CommandBufferSubmitInfo.CommandBuffer = CommandBuffer;
	CommandBufferSubmitInfo.deviceMask = 0;

	// Wait Semaphore Info
	VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo{};
	WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	WaitSemaphoreSubmitInfo.pNext = nullptr;
	WaitSemaphoreSubmitInfo.semaphore = CurrentFrame.SwapchainSemaphore;
	WaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
	WaitSemaphoreSubmitInfo.deviceIndex = 0;
	WaitSemaphoreSubmitInfo.value = 0; // Should be set to 0 for binary semaphores

	// Signal Semaphore Info
	VkSemaphoreSubmitInfo SignalSemaphoreSubmitInfo{};
	SignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	SignalSemaphoreSubmitInfo.pNext = nullptr;
	SignalSemaphoreSubmitInfo.semaphore = CurrentFrame.RenderSemaphore;
	SignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	SignalSemaphoreSubmitInfo.deviceIndex = 0;
	SignalSemaphoreSubmitInfo.value = 0; // Should be set to 0 for binary semaphores

	// Submit Info
	VkSubmitInfo2 SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	SubmitInfo.pNext = nullptr;
	SubmitInfo.waitSemaphoreInfoCount = 1;
	SubmitInfo.pWaitSemaphoreInfos = &WaitSemaphoreSubmitInfo;
	SubmitInfo.signalSemaphoreInfoCount = 1;
	SubmitInfo.pSignalSemaphoreInfos = &SignalSemaphoreSubmitInfo;
	SubmitInfo.commandBufferInfoCount = 1;
	SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

	// Submit command buffer to the queue and execute it.
	// RenderFence will now block until the graphic commands finish execution
	Result = vkQueueSubmit2(Device.GraphicsQueue, 1, &SubmitInfo, CurrentFrame.RenderFence);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &Swapchain->SwapchainKHR;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &CurrentFrame.RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &SwapchainImageIndex;

	Result = vkQueuePresentKHR(Device.GraphicsQueue, &presentInfo);

	// increase the number of frames drawn
	CurrentFrameIndex++;
}

void PVulkanRenderer::CreateInstance()
{
	Instance.ValidationLayers.push_back(SValidationLayer("VK_LAYER_KHRONOS_validation"));

	VkApplicationInfo AppInfo{};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Rocket Engine Vulkan Application";
	AppInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	AppInfo.pEngineName = "No Engine";
	AppInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &AppInfo;

	// GLFW required Vulkan extensions
	uint32_t GlfwExtensionCount = 0;
	const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

	std::vector<const char*> Extensions(GlfwExtensions, GlfwExtensions + GlfwExtensionCount);
	Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
	CreateInfo.ppEnabledExtensionNames = Extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

	std::vector<const char*> Result;
	for (auto& ValidationLayer : Instance.ValidationLayers)
	{
		Result.emplace_back(ValidationLayer.Name);
	}
	CreateInfo.enabledLayerCount = Result.size();
	CreateInfo.ppEnabledLayerNames = Result.data();

	DebugCreateInfo = CreateDebugMessengerCreateInfo();
	CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;

	VkInstance Handle;
	VkResult CreateInstanceResult = vkCreateInstance(&CreateInfo, nullptr, &Handle);
	RK_ENGINE_ASSERT(CreateInstanceResult == VK_SUCCESS, "Failed to create Vulkan context.");
	Instance.Handle = Handle;

	SetupDebugMessenger();
}

void PVulkanRenderer::CreateSurfaceInterface()
{
	VkWin32SurfaceCreateInfoKHR CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	CreateInfo.hwnd = glfwGetWin32Window((GLFWwindow*)GetWindow()->GetNativeWindow());
	CreateInfo.hinstance = GetModuleHandle(nullptr);

	VkResult Result;
	Result = glfwCreateWindowSurface(Instance.Handle, (GLFWwindow*)GetWindow()->GetNativeWindow(), nullptr, &Instance.SurfaceInterface);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Win32 surface");
}

void PVulkanRenderer::SetupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();
	VkResult Result = VkUtility::CreateDebugUtilsMessengerEXT(Instance.Handle, &createInfo, nullptr, &Instance.DebugMessenger);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS);
}

void PVulkanRenderer::DestroyDebugMessenger()
{
	VkUtility::DestroyDebugUtilsMessengerEXT(Instance.Handle, Instance.DebugMessenger, nullptr);
}

VkDebugUtilsMessengerCreateInfoEXT PVulkanRenderer::CreateDebugMessengerCreateInfo()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	return createInfo;
}

VkDebugUtilsMessengerEXT PVulkanRenderer::GetDebugMessenger() const
{
	return Instance.DebugMessenger;
}

VKAPI_ATTR VkBool32 VKAPI_CALL PVulkanRenderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
	switch (Severity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	{ RK_ENGINE_VERBOSE(CallbackData->pMessage);break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		{ RK_ENGINE_INFO(CallbackData->pMessage);   break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	{ RK_ENGINE_WARNING(CallbackData->pMessage);break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		{ RK_ENGINE_ERROR(CallbackData->pMessage);  break; }
	}
	return VK_FALSE;
}

void PVulkanRenderer::Resize()
{
	WaitUntilIdle();

	GetSwapchain()->RegenerateSwapchain(GetInstance().SurfaceInterface, GetDevice().PhysicalDevice, GetDevice().LogicalDevice, GetAllocator());
	CreateGlobalDescriptorAllocator();


	StandardGraphicsPipeline->Free(Device.LogicalDevice);
	StandardComputePipeline->Free(Device.LogicalDevice);
	
	uploadmesh(StandardGraphicsPipeline->MeshBuffer);
	
	StandardComputePipeline->Init(Device.LogicalDevice, Swapchain->DrawImage.imageView, GlobalDescriptorAllocator.Pool, Swapchain->DrawImage.imageFormat);
	StandardGraphicsPipeline->Init(Device.LogicalDevice, Swapchain->DrawImage.imageView, GlobalDescriptorAllocator.Pool, Swapchain->DrawImage.imageFormat);
}

void PVulkanRenderer::CreatePhysicalDevice()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance.Handle, &DeviceCount, 0);
	RK_ENGINE_ASSERT(DeviceCount > 0, "No physical device found.");

	std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(Instance.Handle, &DeviceCount, PhysicalDevices.data());

	for (VkPhysicalDevice PhysicalDevice : PhysicalDevices)
	{
		std::optional<uint32_t> Indices;
		if (Vulkan::Utility::IsVulkanCapableDevice(Instance.SurfaceInterface, PhysicalDevice, Instance.Extensions))
		{
			Device.PhysicalDevice = PhysicalDevice;
			break;
		}
	}
}

void PVulkanRenderer::CreateLogicalDevice()
{
	SQueueFamilyIndices QueueFamilyIndices = Vulkan::Utility::RequestQueueFamilies(Instance.SurfaceInterface, Device.PhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	std::set<uint32_t> UniqueQueueFamilies{ QueueFamilyIndices.GraphicsFamily.value(), QueueFamilyIndices.PresentFamily.value() };

	float QueuePriority = 1.0f;
	for (uint32_t Idx : UniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo QueueCreateInfo{};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = Idx;
		QueueCreateInfo.queueCount = 1;
		QueueCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfos.push_back(QueueCreateInfo);
	}

	// Vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features Features_1_3{};
	Features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	Features_1_3.dynamicRendering = VK_TRUE;
	Features_1_3.synchronization2 = VK_TRUE;

	// Vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features Features_1_2{};
	Features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	Features_1_2.bufferDeviceAddress = VK_TRUE;
	Features_1_2.descriptorIndexing = VK_TRUE;

	// Chain the features together
	Features_1_3.pNext = &Features_1_2;

	VkPhysicalDeviceFeatures DeviceFeatures{};

	VkDeviceCreateInfo CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	CreateInfo.pEnabledFeatures = &DeviceFeatures;
	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Instance.Extensions.size());
	CreateInfo.ppEnabledExtensionNames = Instance.Extensions.data();
	CreateInfo.pNext = &Features_1_3;

	std::vector<const char*> Result;
	for (auto& ValidationLayer : Instance.ValidationLayers)
	{
		Result.emplace_back(ValidationLayer.Name);
	}
	CreateInfo.enabledLayerCount = static_cast<uint32_t>(Result.size());
	CreateInfo.ppEnabledLayerNames = Result.data();

	VkResult CreateLogicalDeviceResult = vkCreateDevice(Device.PhysicalDevice, &CreateInfo, nullptr, &Device.LogicalDevice);
	RK_ENGINE_ASSERT(CreateLogicalDeviceResult == VK_SUCCESS, "Failed to create logical device.");

	vkGetDeviceQueue(Device.LogicalDevice, QueueFamilyIndices.GraphicsFamily.value(), 0, &Device.GraphicsQueue);
	vkGetDeviceQueue(Device.LogicalDevice, QueueFamilyIndices.PresentFamily.value(), 0, &Device.PresentQueue);

	Device.GraphicsFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();
}

void PVulkanRenderer::CreateCommandPool()
{
	// Create a command pool for commands submitted to the graphics queue.
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = Device.GraphicsFamilyIndex;

	VkResult Result;
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		Result = vkCreateCommandPool(Device.LogicalDevice, &commandPoolInfo, nullptr, &Frames[i].CommandPool);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = Frames[i].CommandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		Result = vkAllocateCommandBuffers(Device.LogicalDevice, &cmdAllocInfo, &Frames[i].CommandBuffer);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers.");
	}

	{
		Result = vkCreateCommandPool(Device.LogicalDevice, &commandPoolInfo, nullptr, &ImmediateCommandPool);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = ImmediateCommandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		Result = vkAllocateCommandBuffers(Device.LogicalDevice, &cmdAllocInfo, &ImmediateCommandBuffer);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create command buffers.");
	}
}

void PVulkanRenderer::CreateSynchronizationObjects()
{
	// One fence to control when the gpu has finished rendering the frame, and 2 semaphores to synchronize rendering with swapchain.
	// We want the fence to start signalled so we can wait on it on the first frame

	VkFenceCreateInfo FenceCreateInfo = {};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.pNext = nullptr;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.pNext = nullptr;
	SemaphoreCreateInfo.flags = 0;

	VkResult Result;
	for (int32_t Index = 0; Index < FRAME_OVERLAP; Index++)
	{
		Result = vkCreateFence(Device.LogicalDevice, &FenceCreateInfo, nullptr, &Frames[Index].RenderFence);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create render fence.");

		Result = vkCreateSemaphore(Device.LogicalDevice, &SemaphoreCreateInfo, nullptr, &Frames[Index].SwapchainSemaphore);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain semaphore.");

		Result = vkCreateSemaphore(Device.LogicalDevice, &SemaphoreCreateInfo, nullptr, &Frames[Index].RenderSemaphore);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create render semaphore.");
	}

	Result = vkCreateFence(Device.LogicalDevice, &FenceCreateInfo, nullptr, &ImmediateFence);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create immediate mode fence.");
}

void PVulkanRenderer::CreateGlobalMemoryAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = Device.PhysicalDevice;
	allocatorInfo.device = Device.LogicalDevice;
	allocatorInfo.instance = Instance.Handle;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &GlobalMemoryAllocator);
}

void PVulkanRenderer::CreateGlobalDescriptorAllocator()
{
	static bool bGlobalInitFirstTime = false;

	// Cleanup old descriptor set and layout (if they exist)
	//if (ComputeDescriptorSetLayout != VK_NULL_HANDLE)
	//{
	//	vkDestroyDescriptorSetLayout(Device.LogicalDevice, ComputeDescriptorSetLayout, nullptr);
	//	ComputeDescriptorSetLayout = VK_NULL_HANDLE;
	//}
	
	//if (MeshDescriptorSetLayout != VK_NULL_HANDLE)
	//{
	//	vkDestroyDescriptorSetLayout(Device.LogicalDevice, MeshDescriptorSetLayout, nullptr);
	//	MeshDescriptorSetLayout = VK_NULL_HANDLE;
	//}
	
	GlobalDescriptorAllocator.ClearDescriptors(Device.LogicalDevice);
	if (!bGlobalInitFirstTime)
	{
		// Reset the descriptor pool, which effectively frees all descriptor sets allocated from it

		// Create a descriptor pool that will hold 10 sets with 1 image each
		std::vector<SDescriptorAllocator::SPoolSizeRatio> Sizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		};

		// Initialize or reset the descriptor pool
		GlobalDescriptorAllocator.InitializePool(Device.LogicalDevice, 10, Sizes);

		bGlobalInitFirstTime = true;
	}

	//{
	//	SDescriptorLayoutBuilder builder;
	//	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	//	ComputeDescriptorSetLayout = builder.Build(Device.LogicalDevice, VK_SHADER_STAGE_COMPUTE_BIT);
	//
	//	// Allocate a descriptor set for the draw image
	//	ComputeDescriptorSet = GlobalDescriptorAllocator.Allocate(Device.LogicalDevice, ComputeDescriptorSetLayout);
	//
	//	// Update the descriptor set with the new image view from the swapchain
	//	VkDescriptorImageInfo ImageInfo{};
	//	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	//	ImageInfo.imageView = Swapchain->DrawImage.imageView;  // Use the new image view after swapchain recreation
	//
	//	VkWriteDescriptorSet RenderTargetWrite = {};
	//	RenderTargetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	RenderTargetWrite.pNext = nullptr;
	//	RenderTargetWrite.dstBinding = 0;
	//	RenderTargetWrite.dstSet = ComputeDescriptorSet;
	//	RenderTargetWrite.descriptorCount = 1;
	//	RenderTargetWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	//	RenderTargetWrite.pImageInfo = &ImageInfo;
	//
	//	vkUpdateDescriptorSets(Device.LogicalDevice, 1, &RenderTargetWrite, 0, nullptr);
	//}
}

void PVulkanRenderer::uploadmesh(SMeshBuffer& MeshBuffer)
{
	//> init_data
	std::array<SVertex, 4> rect_vertices;

	rect_vertices[0].position = { 0.5,-0.5, 0 };
	rect_vertices[1].position = { 0.5,0.5, 0 };
	rect_vertices[2].position = { -0.5,-0.5, 0 };
	rect_vertices[3].position = { -0.5,0.5, 0 };

	rect_vertices[0].color = { 0,0, 0,1 };
	rect_vertices[1].color = { 0.5,0.5,0.5 ,1 };
	rect_vertices[2].color = { 1,0, 0,1 };
	rect_vertices[3].color = { 0,1, 0,1 };

	std::array<uint32_t, 6> rect_indices;

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;

	MeshBuffer = CreateMeshBuffer(rect_indices, rect_vertices);
}

void PVulkanRenderer::CreatePipeline()
{
	if (!StandardComputePipeline)
	{
		StandardComputePipeline = new PVulkanStandardComputePipeline(GlobalMemoryAllocator);
	}
	StandardComputePipeline->Init(Device.LogicalDevice, Swapchain->DrawImage.imageView, GlobalDescriptorAllocator.Pool, Swapchain->DrawImage.imageFormat);

	if (!StandardGraphicsPipeline)
	{
		StandardGraphicsPipeline = new PVulkanColorGraphicsPipeline(GlobalMemoryAllocator);
	}
	uploadmesh(StandardGraphicsPipeline->MeshBuffer);
	StandardGraphicsPipeline->Init(Device.LogicalDevice, Swapchain->DrawImage.imageView, GlobalDescriptorAllocator.Pool, Swapchain->DrawImage.imageFormat);
}

void PVulkanRenderer::InitImGui()
{
	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize pool_sizes[] = { 
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkResult Result = vkCreateDescriptorPool(Device.LogicalDevice, &pool_info, nullptr, &ImGuiDescriptorPool);

	// Initialize the pool with the required sizes

	// Initialize ImGui context and Vulkan integration
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)GetWindow()->GetNativeWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Instance.Handle;
	init_info.PhysicalDevice = Device.PhysicalDevice;
	init_info.Device = Device.LogicalDevice;
	init_info.Queue = Device.GraphicsQueue;
	init_info.DescriptorPool = ImGuiDescriptorPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &Swapchain->SwapchainImageFormat;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();
}

void PVulkanRenderer::DrawImGui(VkCommandBuffer CommandBuffer, VkClearValue* ClearValue, VkImageView ImageView)
{
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.pNext = nullptr;

	colorAttachment.imageView = ImageView;
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = ClearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	if (ClearValue)
	{
		colorAttachment.clearValue = *ClearValue;
	}

	VkRenderingInfo renderInfo = {};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.pNext = nullptr;
	renderInfo.flags = 0;
	renderInfo.renderArea.offset = { 0, 0 };
	renderInfo.renderArea.extent = Swapchain->SwapchainExtent;  // Assuming _swapchainExtent is of type VkExtent2D
	renderInfo.layerCount = 1;
	renderInfo.viewMask = 0;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachment;
	renderInfo.pDepthAttachment = nullptr;  // Set this if you have a depth attachment
	renderInfo.pStencilAttachment = nullptr;  // Set this if you have a stencil attachment
	renderInfo.pNext = nullptr;

	vkCmdBeginRendering(CommandBuffer, &renderInfo);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);
	vkCmdEndRendering(CommandBuffer);
}

SBuffer PVulkanRenderer::CreateBuffer(size_t AllocSize, VmaMemoryUsage MemoryUsage, VkBufferUsageFlags UsageFlags)
{
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = AllocSize;
	bufferInfo.usage = UsageFlags;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = MemoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	SBuffer newBuffer;

	VkResult Result = vmaCreateBuffer(GlobalMemoryAllocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");

	return newBuffer;
}

SMeshBuffer PVulkanRenderer::CreateMeshBuffer(std::span<uint32_t> indices, std::span<SVertex> vertices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(SVertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	SMeshBuffer newSurface;

	//create vertex buffer
	newSurface.vertexBuffer = CreateBuffer(vertexBufferSize, VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	//find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = newSurface.vertexBuffer.buffer };
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(Device.LogicalDevice, &deviceAdressInfo);

	//create index buffer
	newSurface.indexBuffer = CreateBuffer(indexBufferSize, VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	
	SBuffer staging = CreateBuffer(vertexBufferSize + indexBufferSize, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data = staging.allocation->GetMappedData();

	// copy vertex buffer
	memcpy(data, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

	ImmediateSubmit([&](VkCommandBuffer cmd)
	{
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	DestroyBuffer(staging);

	return newSurface;
}

void PVulkanRenderer::DestroyBuffer(const SBuffer& Buffer)
{
	vmaDestroyBuffer(GlobalMemoryAllocator, Buffer.buffer, Buffer.allocation);
}

void PVulkanRenderer::WaitUntilIdle()
{
	vkDeviceWaitIdle(Device.LogicalDevice);
}

void PVulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer CommandBuffer)>&& Func)
{
	VkResult Result;
	Result = vkResetFences(Device.LogicalDevice, 1, &ImmediateFence);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to reset fence.");
	
	Result = vkResetCommandBuffer(ImmediateCommandBuffer, 0);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");

	VkCommandBuffer CommandBuffer = ImmediateCommandBuffer;
	
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.pInheritanceInfo = nullptr;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	Result = vkBeginCommandBuffer(CommandBuffer, &info);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to begin command buffer.");

	Func(CommandBuffer);

	Result = vkEndCommandBuffer(CommandBuffer);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to end command buffer..");

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo{};
	CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	CommandBufferSubmitInfo.pNext = nullptr;
	CommandBufferSubmitInfo.CommandBuffer = CommandBuffer;
	CommandBufferSubmitInfo.deviceMask = 0;

    VkSubmitInfo2 SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	SubmitInfo.pNext = nullptr;
	SubmitInfo.waitSemaphoreInfoCount = 0;
	SubmitInfo.pWaitSemaphoreInfos = nullptr;
	SubmitInfo.signalSemaphoreInfoCount = 0;
	SubmitInfo.pSignalSemaphoreInfos = nullptr;
	SubmitInfo.commandBufferInfoCount = 1;
	SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	Result = vkQueueSubmit2(Device.GraphicsQueue, 1, &SubmitInfo, ImmediateFence);
	Result = vkWaitForFences(Device.LogicalDevice, 1, &ImmediateFence, true, 9999999999);
}