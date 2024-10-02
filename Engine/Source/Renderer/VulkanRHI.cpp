#include "EnginePCH.h"
#include "VulkanRHI.h"

#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanRHI::Init()
{
	Instance = new PVulkanInstance();
	Device = new PVulkanDevice();
	SceneRenderer = new PVulkanSceneRenderer();
	
	Instance->Init();
	Device->Init();
	SceneRenderer->Init();
}

void PVulkanRHI::Shutdown()
{
	vkDeviceWaitIdle(Device->GetVkDevice());

	SceneRenderer->Shutdown();
  	Device->Shutdown();
  	Instance->Shutdown();

  	delete SceneRenderer;
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

PVulkanSceneRenderer* PVulkanRHI::GetSceneRenderer() const
{
	return SceneRenderer;
}
