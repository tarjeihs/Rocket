#pragma once

#include <type_traits>

template<typename TClassTo>
static inline TClassTo* Cast(void* Pointer)
{
    if constexpr (std::is_convertible_v<void*, TClassTo*>)
    {
        return static_cast<TClassTo*>(Pointer);
    }
    else
    {
        return reinterpret_cast<TClassTo*>(Pointer);
    }
}