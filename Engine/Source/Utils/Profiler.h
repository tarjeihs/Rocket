#pragma once

class PProfiler
{
public:
    static void Flush();

    static void StartEventScope(const char* Name);
    static void EndEventScope(const char* Name);
};

struct FProfilerEventScope
{
    FProfilerEventScope(const char* InName)
        : Name(InName)
    {
        PProfiler::StartEventScope(Name);
    }

    ~FProfilerEventScope()
    {
        PProfiler::EndEventScope(Name);
    }

    const char* Name;
};

#if RK_PROFILE
    #define PROFILE_FUNC_SCOPE(Name)    FProfilerEventScope      _PROFILE_FUNC_SCOPE_(Name);
#else
    #define PROFILE_FUNC_SCOPE(...)
#endif