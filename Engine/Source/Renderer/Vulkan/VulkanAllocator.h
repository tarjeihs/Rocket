#pragma once

class FVkAllocator
{
public:
    void Init();
    void Shutdown();

    VmaAllocator GetMemoryAllocator() const;

private:
    VmaAllocator MemoryAllocator;
};