#pragma once

#include <chrono>

struct STimestep
{
	using SClock = std::chrono::high_resolution_clock;
	using STimePoint = std::chrono::time_point<SClock>;
	using SDuration = std::chrono::duration<float>;

	STimePoint Start;
	STimePoint Current;
	SDuration DeltaTime;

	STimestep()
	{
		Start = SClock::now();
		Current = Start;
		DeltaTime = SDuration(0);
	}

	void Validate()
	{
		auto NewTime = SClock::now();
		DeltaTime = NewTime - Current;
		Current = NewTime;
	}

	inline float GetDeltaTime() const
	{
		return DeltaTime.count();
	}

	inline float GetElapsedTime() const
	{
		return std::chrono::duration<float>(Current - Start).count();
	}

	inline float GetStartTime() const
	{
		return std::chrono::duration<float>(Start.time_since_epoch()).count();
	}
};