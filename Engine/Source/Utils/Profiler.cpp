#include "EnginePCH.h"
#include "Profiler.h"

std::stack<SEventData> EventStack;
Json::Value Events;

void PProfiler::StartEvent()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();

    SEventData EventData = { STimer(), __FUNCTION__, oss.str() };
    EventStack.push(EventData);

    Json::Value Event;
    Event["name"] = EventData.Name;
    Event["ph"] = "B";
    Event["ts"] = GetEngine()->Time.GetElapsedTimeAsSeconds();
    Event["pid"] = getpid();
    Event["tid"] = EventData.ThreadID;
    
    Events.append(Event);
}

void PProfiler::EndEvent()
{
    SEventData EventData = EventStack.top();
    EventStack.pop();

    Json::Value Event;
    Event["name"] = EventData.Name;
    Event["ph"] = "E";
    Event["ts"] = GetEngine()->Time.GetElapsedTimeAsSeconds();
    Event["pid"] = getpid();
    Event["tid"] = EventData.ThreadID;

    Events.append(Event);
}

void PProfiler::Save()
{
    Json::Value Trace;
    Trace["traceEvents"] = Events;
    Trace["displayTimeUnit"] = "s";

    std::ofstream File("trace.json");
    File << Trace.toStyledString();
    File.close();
}

void PProfiler::StartEventBlock(const char* Name)
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();

    Json::Value Event;

    Event["name"] = Name;
    Event["ph"] = "B";
    Event["ts"] = static_cast<Json::Value::Int64>(GetEngine()->Time.GetElapsedTimeAsMicroseconds());
    Event["pid"] = getpid();
    Event["tid"] = 0;
    
    Events.append(Event);
}

void PProfiler::EndEventBlock(const char* Name)
{
    Json::Value Event;

    Event["name"] = Name;
    Event["ph"] = "E";
    Event["ts"] = static_cast<Json::Value::Int64>(GetEngine()->Time.GetElapsedTimeAsMicroseconds());
    Event["pid"] = getpid();
    Event["tid"] = 0;

    Events.append(Event);
}