#pragma once

#include "Renderer/Common/RHICommandList.h"

class CVulkanCommandList : public IRHICommandList
{
public:
    virtual void Enqueue(std::function<void()>&& Command) override;
    virtual void Execute() override;
    virtual void Reset() override;

private:
    TArray<std::function<void()>> CommandList;
};