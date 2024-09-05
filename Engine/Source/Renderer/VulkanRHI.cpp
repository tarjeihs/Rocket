#include "VulkanRHI.h"

#include <GLFW/glfw3.h>

#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanRHI::Init()
{
	Instance = new PVulkanInstance(this);
	Instance->Init();

	Device = new PVulkanDevice(this);
	Device->Init();

	Memory = new PVulkanMemory(this);
	Memory->Init();

	SceneRenderer = new PVulkanSceneRenderer(this);
	SceneRenderer->Init();
}

void PVulkanRHI::Shutdown()
{
	vkDeviceWaitIdle(Device->GetVkDevice());

	SceneRenderer->Shutdown();
  	Memory->Shutdown();
  	Device->Shutdown();
  	Instance->Shutdown();

  	delete SceneRenderer;
	delete Memory;
	delete Device;
	delete Instance;
}

void PVulkanRHI::Resize()
{
	vkDeviceWaitIdle(Device->GetVkDevice());

	SceneRenderer->Resize();
}

void PVulkanRHI::Render()
{
	SceneRenderer->Render();
}

PVulkanInstance* PVulkanRHI::GetInstance() const
{
	return Instance;
}

PVulkanDevice* PVulkanRHI::GetDevice() const
{
	return Device;
}

PVulkanMemory* PVulkanRHI::GetMemory() const
{
	return Memory;
}

PVulkanSceneRenderer* PVulkanRHI::GetSceneRenderer() const
{
	return SceneRenderer;
}
