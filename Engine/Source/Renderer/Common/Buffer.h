#pragma once

#include <cstddef>

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual void Allocate(size_t Size) = 0;
    virtual void Free() = 0;
    virtual void Update(const void* Data, size_t Size) = 0;
};