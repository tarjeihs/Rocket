#pragma once

#include <chrono>

using SClock = std::chrono::high_resolution_clock;
using STimePoint = std::chrono::time_point<SClock>;
using SDuration = std::chrono::duration<float>;

struct STimer
{
	STimePoint Start;

	STimer()
	{
		Start = SClock::now();
	}

	inline float GetElapsedTimeAsSeconds() const
	{
		return std::chrono::duration<float>(SClock::now() - Start).count();
	}

	inline float GetElapsedTimeAsMilliseconds() const
    {
        return GetElapsedTimeAsSeconds() * 1e3;
    }

	inline float GetElapsedTimeAsMicroseconds() const
    {
        return GetElapsedTimeAsSeconds() * 1e6;
    }

    inline long long GetElapsedTimeAsNanoseconds() const
    {
        return GetElapsedTimeAsSeconds() * 1e9;
    }
};

struct STimestep
{
	STimer Time;
	STimePoint Current;
	SDuration DeltaTime;

	STimestep()
	{
		DeltaTime = SDuration(0);
	}

	void Reset()
	{
		STimePoint PreviousTime = SClock::now();

		DeltaTime = PreviousTime - Current;

		Current = PreviousTime;
	}

	inline float GetDeltaTime() const
	{
		return DeltaTime.count();
	}
};