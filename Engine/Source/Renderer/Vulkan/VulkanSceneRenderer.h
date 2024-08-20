#pragma once

#include <vector>
#include <functional>
#include <vulkan/vulkan_core.h>

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
		RenderTarget = nullptr;
		DeferredFramePool = nullptr;
		ImmediateFramePool = nullptr;
	}

	void Init();
	void Shutdown();
	void Resize();
	void Render();

	PVulkanSwapchain* GetSwapchain() const;
	PVulkanImage* GetRenderTarget() const;

	void DeferredSubmit();
	void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

protected:
	void PrepareRenderTarget();
	void BindRenderTarget();
	void FinalizeRenderTarget();

protected:
	void TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout CurrentLayout, VkImageLayout NewLayout);
	void CopyImageRegion(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);

private:
	PVulkanSwapchain* Swapchain;
	PVulkanImage* RenderTarget;

	PVulkanFramePool* DeferredFramePool;
	PVulkanFramePool* ImmediateFramePool;

	PVulkanImGui* ImGui;

	class PVulkanGraphicsPipeline* GraphicsPipeline;

private:
	PVulkanRHI* RHI;
};