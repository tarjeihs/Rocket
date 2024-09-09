#pragma once

#include "Core/Logger.h"
#include <chrono>
#include <iostream>

#define STOPWATCH(DebugName) SStopwatch Stopwatch(DebugName)

class SStopwatch
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    explicit SStopwatch(const char* InDebugName)
        : StartTime(Clock::now()), DebugName(InDebugName)
    {
    }

    ~SStopwatch()
    {
        const auto EndTime = Clock::now();
        const auto Elapsed = EndTime - StartTime;

        // Total time in seconds, milliseconds, and nanoseconds
        const long DurationS = std::chrono::duration_cast<std::chrono::seconds>(Elapsed).count();
        const long DurationMS = std::chrono::duration_cast<std::chrono::milliseconds>(Elapsed).count() % 1000;  // Remaining milliseconds after seconds
        const long DurationNS = std::chrono::duration_cast<std::chrono::nanoseconds>(Elapsed).count() % 1000000; // Remaining nanoseconds after milliseconds

        // Log the properly formatted duration
        RK_LOG_TRACE("({}) Elapsed time: {}s:{}ms:{}ns", DebugName, DurationS, DurationMS, DurationNS);    
    }

private:
    TimePoint StartTime;

    const char* DebugName;
};