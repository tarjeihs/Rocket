#pragma once

#include <functional>

#include "Renderer/Common/Renderer.h"
#include "VulkanRenderer.h"

class PVulkanRenderGraph;
class PVulkanRHI;
class PVulkanFramePool;
class FVkImage;
class PVulkanSwapchain;
class PVulkanCommandBuffer;
class PVulkanAllocator;

class PVulkanForwardRenderer : public IRenderer
{
public:
	PVulkanForwardRenderer()
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
	FVkImage* GetDrawImage() const;
	FVkImage* GetDepthImage() const;
	PVulkanRenderGraph* GetRenderGraph() const;
	PVulkanRenderGraph* GetOverlayRenderGraph() const;
	PVulkanFramePool* GetParallelFramePool() const;

	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

private:
	PVulkanAllocator* Allocator;
	PVulkanSwapchain* Swapchain;
	FVkImage* DrawImage; // TODO: Move into Swapchain. This is the final rendering output (color attachment image)
	FVkImage* DepthImage;
	PVulkanRenderGraph* RenderGraph;
	PVulkanRenderGraph* OverlayRenderGraph;
	PVulkanFramePool* ParallelFramePool;
	PVulkanFramePool* ImmediateFramePool;
};