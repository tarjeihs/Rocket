#pragma once

#include <cstdint>
#include <string>
#include <random>

namespace UUID64
{
    static std::random_device RandomDevice;
    static std::mt19937_64 Engine(RandomDevice());
    static std::uniform_int_distribution<uint64_t> UniformDistribution;
}

struct SUUID64
{
    SUUID64()
    {
        UUID = UUID64::UniformDistribution(UUID64::Engine);
    }

    SUUID64(uint32_t InUUID)
    {
        UUID = InUUID;
    }

    inline operator uint64_t() const
    {
        return UUID;
    }

private:
    uint64_t UUID;
};

namespace std
{
    template <typename T> struct hash;

    template<>
    struct hash<SUUID64>
    {
        std::size_t operator()(const SUUID64& UUID) const noexcept
        {
            return (uint64_t)UUID;
        }
    };
};