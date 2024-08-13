#define VMA_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32

#include "EnginePCH.h"
#include "VulkanRenderer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <set>
#include <vector>

#include "Core/Window.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "VulkanUtility.h"

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
	CreateMemoryAllocator();

	Swapchain = new PVulkanSwapchain();
	Swapchain->CreateSwapchain(Instance.SurfaceInterface, Device.PhysicalDevice, Device.LogicalDevice, Allocator);

	CreateCommandPool();
	CreateSynchronizationObjects();

	CreateDescriptorSet();
	CreatePipeline();
}

void PVulkanRenderer::DestroyRenderer()
{
	vkDeviceWaitIdle(Device.LogicalDevice);

	Swapchain->DestroySwapchain(Instance.SurfaceInterface, Device.PhysicalDevice, Device.LogicalDevice, Allocator);

	for (size_t Index = 0; Index < FRAME_OVERLAP; Index++)
	{
		vkDestroySemaphore(Device.LogicalDevice, Frames[Index].RenderSemaphore, nullptr);
		vkDestroySemaphore(Device.LogicalDevice, Frames[Index].SwapchainSemaphore, nullptr);
		vkDestroyFence(Device.LogicalDevice, Frames[Index].RenderFence, nullptr);

		vkDestroyCommandPool(Device.LogicalDevice, Frames[Index].CommandPool, nullptr);
	}

	DescriptorAllocator.DeinitializePool(Device.LogicalDevice);
	vkDestroyDescriptorSetLayout(Device.LogicalDevice, RenderTargetDescriptorSetLayout, nullptr);
	
	vkDestroyPipelineLayout(Device.LogicalDevice, _gradientPipelineLayout, nullptr);
	vkDestroyPipeline(Device.LogicalDevice, _gradientPipeline, nullptr);

	vmaDestroyAllocator(Allocator);
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
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->RenderTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	ClearBackground(CommandBuffer);

	//transition the draw image and the swapchain image into their correct transfer layouts
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->RenderTarget.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::Utility::TransitionImage(CommandBuffer, Swapchain->SwapchainImages[SwapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	Vulkan::Utility::CopyImageToImage(CommandBuffer, Swapchain->RenderTarget.image, Swapchain->SwapchainImages[SwapchainImageIndex], Swapchain->RenderTargetExtent, Swapchain->SwapchainExtent);
	
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

	// Chain Vulkan 1.3 features to Vulkan 1.2 features
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

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		VkResult Result = vkCreateCommandPool(Device.LogicalDevice, &commandPoolInfo, nullptr, &Frames[i].CommandPool);
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
}

void PVulkanRenderer::CreateMemoryAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = Device.PhysicalDevice;
	allocatorInfo.device = Device.LogicalDevice;
	allocatorInfo.instance = Instance.Handle;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &Allocator);
}

void PVulkanRenderer::CreateDescriptorSet()
{
	// Cleanup old descriptor set and layout (if they exist)
	if (RenderTargetDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(Device.LogicalDevice, RenderTargetDescriptorSetLayout, nullptr);
		RenderTargetDescriptorSetLayout = VK_NULL_HANDLE;
	
		// Reset the descriptor pool, which effectively frees all descriptor sets allocated from it
		DescriptorAllocator.ClearDescriptors(Device.LogicalDevice);
	}

	// Create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<SDescriptorAllocator::SPoolSizeRatio> Sizes =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	// Initialize or reset the descriptor pool
	DescriptorAllocator.InitializePool(Device.LogicalDevice, 10, Sizes);

	// Create the descriptor set layout for the compute draw
	{
		SDescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		RenderTargetDescriptorSetLayout = builder.Build(Device.LogicalDevice, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	// Allocate a descriptor set for the draw image
	RenderTargetDescriptorSet = DescriptorAllocator.Allocate(Device.LogicalDevice, RenderTargetDescriptorSetLayout);

	// Update the descriptor set with the new image view from the swapchain
	VkDescriptorImageInfo ImageInfo{};
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	ImageInfo.imageView = Swapchain->RenderTarget.imageView;  // Use the new image view after swapchain recreation

	VkWriteDescriptorSet RenderTargetWrite = {};
	RenderTargetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	RenderTargetWrite.pNext = nullptr;
	RenderTargetWrite.dstBinding = 0;
	RenderTargetWrite.dstSet = RenderTargetDescriptorSet;
	RenderTargetWrite.descriptorCount = 1;
	RenderTargetWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	RenderTargetWrite.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets(Device.LogicalDevice, 1, &RenderTargetWrite, 0, nullptr);
}

void PVulkanRenderer::CreatePipeline()
{
	CreateBackgroundPipeline();
}

void PVulkanRenderer::CreateBackgroundPipeline()
{
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &RenderTargetDescriptorSetLayout;
	computeLayout.setLayoutCount = 1;

	VkResult Result = vkCreatePipelineLayout(Device.LogicalDevice, &computeLayout, nullptr, &_gradientPipelineLayout);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");

	PVulkanShader* Shader = new PVulkanShader();
	SShaderProgram ShaderProgram = Shader->Compile(Device.LogicalDevice, RK_SHADERTYPE_COMPUTE_SHADER, L"../Engine/Shaders/Gradient.compute", L"main", "cs_6_0");

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = _gradientPipelineLayout;
	computePipelineCreateInfo.stage = ShaderProgram.CreateInfo;

	Result = vkCreateComputePipelines(Device.LogicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &_gradientPipeline);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create compute pipeline.");

	Shader->Free(Device.LogicalDevice, ShaderProgram);
}

void PVulkanRenderer::WaitUntilIdle()
{
	vkDeviceWaitIdle(Device.LogicalDevice);
}

void PVulkanRenderer::ClearBackground(VkCommandBuffer CommandBuffer)
{
	// bind the gradient drawing compute pipeline
	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);

	// bind the descriptor set containing the draw image for the compute pipeline
	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &RenderTargetDescriptorSet, 0, nullptr);

	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	vkCmdDispatch(CommandBuffer, std::ceil(Swapchain->RenderTargetExtent.width / 16.0), std::ceil(Swapchain->RenderTargetExtent.height / 16.0), 1);
}