#pragma once

#include <cstdint>
#include <string>

#include "EngineTypes.h"
#include "Types/String.h"

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
inline uint32_t FNV1aHash(const FString& String)
{
    constexpr uint32_t FNVPrime = 16777619u;
    uint32_t Hash = 2166136261u;

    for (SizeType i = 0; i < String.GetSize(); ++i) {
        Hash ^= static_cast<uint32_t>(String.GetData()[i]);
        Hash *= FNVPrime;
    }

    return Hash;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
inline uint32_t FNV1aHash(const FStringView& StringView)
{
    constexpr uint32_t FNVPrime = 16777619u;
    uint32_t Hash = 2166136261u;

    for (SizeType i = 0; i < StringView.GetSize(); ++i) {
        Hash ^= static_cast<uint32_t>(StringView.GetData()[i]);
        Hash *= FNVPrime;
    }

    return Hash;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
inline uint32_t FNV1aHash(const std::string& String)
{
    constexpr uint32_t FNVPrime = 16777619u;
    uint32_t Hash = 2166136261u;

    for (char Character : String) {
        Hash ^= static_cast<uint32_t>(Character);
        Hash *= FNVPrime;
    }

    return Hash;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
inline uint32_t FNV1aHash(const uint32_t Value) 
{
    constexpr uint32_t FNVPrime = 16777619u;
    uint32_t Hash = 2166136261u;

    Hash ^= Value;
    Hash *= FNVPrime;

    return Hash;
}