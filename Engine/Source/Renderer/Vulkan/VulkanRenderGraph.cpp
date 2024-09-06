#include "EnginePCH.h"
#include "VulkanRenderGraph.h"

#include "Renderer/Vulkan/VulkanFrame.h"

void PVulkanRenderGraph::AddCommand(std::function<void(PVulkanFrame*)>&& Func)
{
    Commands.push_back(std::move(Func));
}

void PVulkanRenderGraph::Execute(PVulkanFrame* Frame)
{
    for (const auto& Command : Commands)
    {
        Command(Frame);
    }
}