#pragma once

#include "Core/Assert.h"
#include "EngineTypes.h"

template<typename T>
class TOptional
{
public:
    TOptional()
        : bValid(false)
    {
    }

    TOptional(const T& Value)
        : bValid(true), Storage(Value)
    {
    }

    void Reset()
    {
        bValid = false;
    }

    bool IsValid() const
    {
        return bValid;
    }

    T& Value()
    {
        //RK_ASSERT(bValid == true, "Attempting to access an empty value.");
        return Storage;
    }

    const T& Value() const
    {
        //RK_ASSERT(bValid == true, "Attempting to access an empty value.");
        return Storage;
    }

    operator T&()
    {
        //RK_ASSERT(bValid == true, "Attempting to access an empty value.");
        return Storage;
    }

    operator const T&() const
    {
        //RK_ASSERT(bValid == true, "Attempting to access an empty value.");
        return Storage;
    }

    T& operator*()
    {
        return Storage;
    }

    const T& operator*() const
    {
        return Storage;
    }

    T* operator->()
    {
        return &Storage;
    }

    const T* operator->() const
    {
        return &Storage;
    }

private:
    uint8 bValid : 1;
    
    T Storage;
};