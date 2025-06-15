#include "EnginePCH.h"
#include "VulkanRHI.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanViewport.h"
#include "Renderer/Vulkan/VulkanCommandList.h"

void CVulkanRHI::Init()
{
	Device = MakeUnique<CVulkanDevice>();
	Viewport = MakeUnique<CVulkanViewport>();

	Device->CreateInstance();
	Viewport->CreateViewport(Device.Get());

	Device->CreateDevice(Viewport.Get());
	Viewport->CreateSwapchain(Device.Get());
}

void CVulkanRHI::Shutdown()
{
	Device->WaitUntilIdle();

	Viewport->FreeSwapchain(Device.Get());
	Viewport->FreeViewport(Device.Get());

	Device->FreeDevice();
	Device->FreeInstance();
}

void CVulkanRHI::Resize()
{
	Device->WaitUntilIdle();

	Viewport->FreeSwapchain(Device.Get());
	Viewport->CreateSwapchain(Device.Get());
}

void CVulkanRHI::Render()
{

}

IRHIDevice* CVulkanRHI::GetDevice() const
{
	return Device.Get();
}

IRHIViewport* CVulkanRHI::GetViewport() const
{
	return Viewport.Get();
}

IRHICommandList* CVulkanRHI::GetCommandList() const
{
	return CommandList.Get();
}