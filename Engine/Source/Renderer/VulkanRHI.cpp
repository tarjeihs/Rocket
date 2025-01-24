#include "EnginePCH.h"
#include "VulkanRHI.h"

#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
//#include "Renderer/Vulkan/VkForwardRenderer.h"
#include "Vulkan/VkRenderer.h"

void PVulkanRHI::Init()
{
#if VALIDATION_LAYER
	ExtensionFamily.ValidationLayerExtensions.push_back("VK_LAYER_KHRONOS_validation");
	ExtensionFamily.InstanceExtensions.push_back("VK_EXT_debug_utils");
#endif

	Instance = new FVkInstance();
	Device = new FVkDevice();
	Allocator = new FVkAllocator();
	//Renderer = new FVkForwardRenderer();
	Renderer = new FVkRenderer();

	Instance->Init();
	Device->Init();
	Allocator->Init();
	Renderer->Init();
}

void PVulkanRHI::Shutdown()
{
	vkDeviceWaitIdle(Device->GetVkDevice());

	Renderer->Shutdown();
	Allocator->Shutdown();
  	Device->Shutdown();
  	Instance->Shutdown();

	delete Renderer;
	delete Allocator;
	delete Device;
	delete Instance;
}

void PVulkanRHI::Resize()
{
	vkDeviceWaitIdle(Device->GetVkDevice());

	Renderer->Resize();
}

void PVulkanRHI::Render()
{
	Renderer->Render();
}