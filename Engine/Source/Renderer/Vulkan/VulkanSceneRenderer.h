#pragma once

#include <functional>

#include "Renderer/Common/Renderer.h"

class PVulkanRenderGraph;
class PVulkanRHI;
class PVulkanFramePool;
class PVulkanImage;
class PVulkanSwapchain;
class PVulkanCommandBuffer;
class PVulkanAllocator;

class PVulkanSceneRenderer : public IRenderer
{
public:
	PVulkanSceneRenderer()
	{
		Swapchain = nullptr;
		DrawImage = nullptr;
		ParallelFramePool = nullptr;
		ImmediateFramePool = nullptr;
	}

	void Init();
	void Shutdown();
	void Resize();
	void Render();

	PVulkanAllocator* GetAllocator() const;
	PVulkanSwapchain* GetSwapchain() const;
	PVulkanImage* GetDrawImage() const;
	PVulkanImage* GetDepthImage() const;
	PVulkanRenderGraph* GetRenderGraph() const;
	PVulkanRenderGraph* GetOverlayRenderGraph() const;
	PVulkanFramePool* GetParallelFramePool() const;

	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

private:
	PVulkanAllocator* Allocator;
	PVulkanSwapchain* Swapchain;
	PVulkanImage* DrawImage;
	PVulkanImage* DepthImage;
	PVulkanRenderGraph* RenderGraph;
	PVulkanRenderGraph* OverlayRenderGraph;
	PVulkanFramePool* ParallelFramePool;
	PVulkanFramePool* ImmediateFramePool;
};
