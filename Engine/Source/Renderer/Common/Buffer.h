#pragma once

#include <cstddef>

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual void Allocate(size_t Size) = 0;
    virtual void Free() = 0;
    virtual void Submit(const void* Data, size_t Size, size_t Offset = 0) = 0;
};