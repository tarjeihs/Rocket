#pragma once

#include <vulkan/vulkan_core.h>

class PVulkanRHI;
class PVulkanCommandBuffer;
class PVulkanDescriptorPool;

class PVulkanImGui
{
public:
	void Init();
	void Shutdown();
	void Bind(PVulkanCommandBuffer* CommandBuffer, VkImageView ImageView);

private:
	PVulkanDescriptorPool* DescriptorPool;
};