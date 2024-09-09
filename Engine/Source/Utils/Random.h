#pragma once

#include "Utils/Stopwatch.h"
#include <array>
#include <random>

namespace Random
{
    static std::random_device RandomDevice;
    static std::mt19937_64 Engine(RandomDevice());
}

struct SWELL512
{
    SWELL512()
    {
        for (int32_t Index = 0; Index < 16; ++Index)
        {
            StateArray[Index] = static_cast<uint32_t>(Random::RandomDevice());
        }

        State = 0;
    }

    uint32_t Generate()
    {
        uint32_t A, B, C, D;
        A = StateArray[State];
        C = StateArray[(State + 13) & 15];
        B = A ^ C ^ (A << 16) ^ (C << 15);
        C = StateArray[(State + 9) & 15];
        C ^= (C >> 11);
        A = StateArray[State] = B ^ C;
        D = A ^ ((A << 5) & 0xDA442D24UL);
        State = (State + 15) & 15;
        A = StateArray[State];
        StateArray[State] = A ^ B ^ D ^ (A << 2) ^ (B << 18) ^ (C << 28);
        return StateArray[State];
    }

private:
    std::array<uint32_t, 16> StateArray;
    
    uint32_t State;
};

#define FLOAT_VALUE 1e6f        // 10^6 (1 million)
#define DOUBLE_VALUE 1e15       // 10^15 (1 quadrillion)

#define FLOAT_VALUE_MIN -FLOAT_VALUE
#define FLOAT_VALUE_MAX FLOAT_VALUE

#define DOUBLE_VALUE_MIN -DOUBLE_VALUE
#define DOUBLE_VALUE_MAX DOUBLE_VALUE

struct SRandom
{
    static float GetVal() 
    {
        std::uniform_real_distribution<float> Dis(20'000.0f, 300'000.0f);
        return Dis(Random::Engine);
    }

    static int32_t GetInt32Value()
    {
        thread_local SWELL512 Instance;
        const int32_t Value = static_cast<int32_t>(Instance.Generate());
        return Value;
    }

    static int32_t GetInt32Value(int32_t Min, int32_t Max)
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return Min + NormalizedValue * (Max - Min);
    }

    static uint32_t GetUInt32Value()
    {
        thread_local SWELL512 Instance;
        return Instance.Generate();
    }

    static uint32_t GetUInt32Value(uint32_t Min, uint32_t Max)
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return Min + NormalizedValue * (Max - Min);
    }

    static float GetFloatValue()
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return FLOAT_VALUE_MIN + NormalizedValue * (FLOAT_VALUE_MAX - FLOAT_VALUE_MIN);
    }

    static float GetFloatValue(float Min, float Max)
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return Min + NormalizedValue * (Max - Min);
    }

    static double GetDoubleValue()
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return DOUBLE_VALUE_MIN + NormalizedValue * (DOUBLE_VALUE_MAX - DOUBLE_VALUE_MIN);
    }

    static double GetDoubleValue(double Min, double Max)
    {
        thread_local SWELL512 Instance;
        const double NormalizedValue = static_cast<double>(Instance.Generate()) / static_cast<double>(std::numeric_limits<uint32_t>::max());
        return Min + NormalizedValue * (Max - Min);
    }

    static uint64_t GetUInt64Value()
    {
        thread_local SWELL512 Instance;
        uint64_t Low = static_cast<uint64_t>(Instance.Generate());
        uint64_t High = static_cast<uint64_t>(Instance.Generate());
        return (High << 32) | Low;
    }

    static uint64_t GetUInt64Value(uint64_t Min, uint64_t Max)
    {
        thread_local SWELL512 Instance;
        uint64_t Low = static_cast<uint64_t>(Instance.Generate());
        uint64_t High = static_cast<uint64_t>(Instance.Generate());
        uint64_t Value = (High << 32) | Low;
        double NormalizedValue = static_cast<double>(Value) / static_cast<double>(std::numeric_limits<uint64_t>::max());
        return Min + static_cast<uint64_t>(NormalizedValue * (Max - Min));
    }

    static int64_t GetInt64Value()
    {
        thread_local SWELL512 Instance;
        uint64_t Low = static_cast<uint64_t>(Instance.Generate());
        uint64_t High = static_cast<uint64_t>(Instance.Generate());
        return static_cast<int64_t>((High << 32) | Low);
    }

    static int64_t GetInt64Value(int64_t Min, int64_t Max)
    {
        thread_local SWELL512 Instance;
        int64_t Low = static_cast<int64_t>(Instance.Generate());
        int64_t High = static_cast<int64_t>(Instance.Generate());
        int64_t Value = (High << 32) | Low;
        double NormalizedValue = static_cast<double>(Value) / static_cast<double>(std::numeric_limits<uint64_t>::max());
        return Min + static_cast<int64_t>(NormalizedValue * (Max - Min));
    }
};