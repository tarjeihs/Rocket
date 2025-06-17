#include "EnginePCH.h"
#include "VulkanRHI.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanViewport.h"
#include "Renderer/Vulkan/VulkanCommandList.h"

void CVulkanRHI::Init()
{
	Device = MakeUnique<CVulkanDevice>();
	Device->Initialize();

	Viewports.push_back(MakeUnique<CVulkanViewport>());
	Viewports[0]->Initialize();
}

void CVulkanRHI::Shutdown()
{
	Device->WaitUntilIdle();

	for (const auto& Viewport : Viewports)
	{
		Viewport->Shutdown();
	}

	Device->Shutdown();
}

void CVulkanRHI::Resize()
{
	//Device->WaitUntilIdle();
}

void CVulkanRHI::Render()
{

	for (const auto& Viewport : Viewports)
	{
		Viewport->BeginFrame();
		Viewport->EndFrame();
		Viewport->Present();
	}
}

IRHIDevice* CVulkanRHI::GetDevice()
{
	return Device.Get();
}

IRHIViewport* CVulkanRHI::GetViewport(uint32_t Index)
{
	return Viewports[Index].Get();
}