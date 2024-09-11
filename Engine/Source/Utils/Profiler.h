#pragma once

struct SEventData
{
    STimer Time;
    std::string Name;
    std::string ThreadID;
};

class PProfiler
{
public:
    static void Init();
    static void Save();

    static void StartEvent();
    static void EndEvent();

    static void StartEventBlock(const char* Name);
    static void EndEventBlock(const char* Name);
};

struct SProfilerScope
{
    SProfilerScope(const char* InName)
        : Name(InName)
    {
        PProfiler::StartEventBlock(Name);
    }

    ~SProfilerScope()
    {
        PProfiler::EndEventBlock(Name);
    }

    const char* Name;
};

#define PROFILE_FUNC_SCOPE(Name) SProfilerScope _PROFILE_FUNC_SCOPE_(Name);