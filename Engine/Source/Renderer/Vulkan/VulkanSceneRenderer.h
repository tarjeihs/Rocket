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
	PVulkanSceneRenderer(PVulkanRHI* InRHI)
		: RHI(InRHI)
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
	PVulkanImage* GetRenderTarget() const;
	PVulkanImage* GetDepthImage() const;
	PVulkanRenderGraph* GetRenderGraph() const;
	PVulkanFramePool* GetFramePool() const;

	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

private:
	PVulkanSwapchain* Swapchain;
	PVulkanImage* DrawImage;
	PVulkanImage* DepthImage;
	PVulkanRenderGraph* RenderGraph;
	PVulkanFramePool* DeferredFramePool;
	PVulkanFramePool* ImmediateFramePool;
	PVulkanImGui* ImGui;

private:
	PVulkanRHI* RHI;
};
