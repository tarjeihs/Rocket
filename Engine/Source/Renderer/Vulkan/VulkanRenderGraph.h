#pragma once

#include <functional>

class PVulkanCommandBuffer;

class PVulkanRenderGraph
{
public:
    void AddCommand(std::function<void(PVulkanCommandBuffer*)>&& Func);

    void BeginRendering();
    void Execute(class PVulkanCommandBuffer* CommandBuffer);
    void EndRendering();

private:
    std::vector<std::function<void(PVulkanCommandBuffer*)>> Commands;
};