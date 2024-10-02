#pragma once

class PVulkanAllocator
{
public:
    void Init();
    void Shutdown();

    VmaAllocator GetMemoryAllocator() const;

private:
    VmaAllocator MemoryAllocator;
};