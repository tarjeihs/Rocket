#pragma once

#include <vulkan/vulkan_core.h>

class PVulkanRHI;
class PVulkanCommandBuffer;
class PVulkanDescriptorPool;

class PVulkanImGui
{
public:
	void Init(PVulkanRHI* RHI);
	void Shutdown(PVulkanRHI* RHI);
	void Bind(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer, VkImageView ImageView);

private:
	PVulkanDescriptorPool* DescriptorPool;
};