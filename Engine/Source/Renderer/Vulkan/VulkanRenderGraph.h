#pragma once

#include <functional>

class PVulkanFrame;

class PVulkanRenderGraph
{
public:
    void AddCommand(std::function<void(PVulkanFrame*)>&& Func);

    void Execute(PVulkanFrame* Frame);

private:
    std::vector<std::function<void(PVulkanFrame*)>> Commands;
};