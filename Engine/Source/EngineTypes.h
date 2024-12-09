#pragma once

using SizeType = size_t;

using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;

template<typename TObject>
constexpr typename std::remove_reference<TObject>::type&& MoveTemp(TObject&& Object) 
{
    return static_cast<typename std::remove_reference<TObject>::type&&>(Object);
}