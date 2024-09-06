#pragma once

#include <functional>

#include "Renderer/Common/Renderer.h"

class PVulkanRenderGraph;
class PVulkanRHI;
class PVulkanFramePool;
class PVulkanImage;
class PVulkanSwapchain;
class PVulkanCommandBuffer;

class PVulkanSceneRenderer : public IRenderer
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
	PVulkanRenderGraph* GetOverlayRenderGraph() const;
	PVulkanFramePool* GetFramePool() const;

	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

private:
	PVulkanSwapchain* Swapchain;
	PVulkanImage* DrawImage;
	PVulkanImage* DepthImage;
	PVulkanRenderGraph* RenderGraph;
	PVulkanRenderGraph* OverlayRenderGraph;
	PVulkanFramePool* DeferredFramePool;
	PVulkanFramePool* ImmediateFramePool;
};
