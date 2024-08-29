#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

class PVulkanRenderGraph
{
public:
    void AddCommand(std::function<void(VkCommandBuffer)>&& Func);

    void Execute(VkCommandBuffer CommandBuffer);

private:
    std::vector<std::function<void(VkCommandBuffer)>> Commands;
};