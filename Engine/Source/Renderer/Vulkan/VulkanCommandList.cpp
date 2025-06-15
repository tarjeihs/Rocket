#include "EnginePCH.h"
#include "VulkanCommandList.h"

void CVulkanCommandList::Enqueue(std::function<void()> &&Command)
{
    CommandList.Add(std::move(Command));
}

void CVulkanCommandList::Execute()
{
    for (std::function<void()> &Command : CommandList)
    {
        Command();
    }
}

void CVulkanCommandList::Reset()
{
    CommandList.Clear();
}
