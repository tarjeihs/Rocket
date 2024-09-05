#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

class PVulkanRenderGraph;
class PVulkanRHI;
class PVulkanFramePool;
class PVulkanImage;
class PVulkanImGui;
class PVulkanSwapchain;
class PVulkanCommandBuffer;

class PVulkanSceneRenderer
{
public:
	PVulkanSceneRenderer()
	{
		Swapchain = nullptr;
		DrawImage = nullptr;
		DeferredFramePool = nullptr;
		ImmediateFramePool = nullptr;
	}

	void Init();
	void Shutdown();
	void Resize();
	void Render();

	PVulkanSwapchain* GetSwapchain() const;
	PVulkanImage* GetDrawImage() const;
	PVulkanImage* GetDepthImage() const;
	PVulkanRenderGraph* GetRenderGraph() const;
	PVulkanFramePool* GetFramePool() const;
	PVulkanImGui* GetImGui() const;

	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

private:
	PVulkanSwapchain* Swapchain;
	PVulkanImage* DrawImage;
	PVulkanImage* DepthImage;
	PVulkanRenderGraph* RenderGraph;
	PVulkanFramePool* DeferredFramePool;
	PVulkanFramePool* ImmediateFramePool;
	PVulkanImGui* ImGui;
};
