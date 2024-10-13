#pragma once

#include "Utils/FileSystem.h"

class ITexture2D
{
public:
    ~ITexture2D() = default;

    virtual void CreateTexture2D(unsigned char* Data) = 0;
    virtual void DestroyTexture2D() = 0;
};