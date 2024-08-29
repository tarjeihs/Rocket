#include "EnginePCH.h"
#include "VulkanRenderGraph.h"

void PVulkanRenderGraph::AddCommand(std::function<void(VkCommandBuffer)>&& Func)
{
    Commands.push_back(std::move(Func));
}

void PVulkanRenderGraph::Execute(VkCommandBuffer CommandBuffer)
{
    for (const auto& Command : Commands)
    {
        Command(CommandBuffer);
    }
}
