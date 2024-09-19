#pragma once

#include <functional>

class PVulkanFrame;

class PVulkanRenderGraph
{
public:
    void AddCommand(std::function<void(PVulkanFrame*)>&& Func);

    void BeginRendering();
    void Execute(PVulkanFrame* Frame);
    void EndRendering();

private:
    std::vector<std::function<void(PVulkanFrame*)>> Commands;
};