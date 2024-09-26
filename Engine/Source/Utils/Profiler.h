#pragma once

class PProfiler
{
public:
    static void Flush();

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

#ifdef RK_PROFILE
#define PROFILE_FUNC_SCOPE(Name) SProfilerScope _PROFILE_FUNC_SCOPE_(Name);
#else
#define PROFILE_FUNC_SCOPE(...)
#endif