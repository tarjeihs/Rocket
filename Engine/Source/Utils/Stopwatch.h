#pragma once

#include <chrono>
#include <iostream>

#include "Core/Logger.h"

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
        const long DurationS = std::chrono::duration_cast<std::chrono::seconds>(EndTime - StartTime).count();
        const long DurationMS = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime).count();
        const long DurationNS = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - StartTime).count();
        RK_LOG_TRACE("({}) Elapsed time: {}s:{}ms:{}ns", DebugName, DurationS, DurationMS, DurationNS);
    }

private:
    TimePoint StartTime;

    const char* DebugName;
};