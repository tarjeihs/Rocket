#include "EnginePCH.h"
#include "Profiler.h"

#include <fstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

struct SEvent
{
    const char* Name;
    const char* Phase;
    int64_t Timestamp;
    uint32_t ThreadID;
    uint32_t ProcessID;

    bool IsComplete() const
    {
        return std::strcmp(Phase, "E") == 0;
    }
};

std::vector<SEvent> Timeline;

void PProfiler::Flush()
{
    rapidjson::Document Document;
    Document.SetObject();
    rapidjson::Document::AllocatorType& Allocator = Document.GetAllocator();

    rapidjson::Value TraceEvents(rapidjson::kArrayType);
    for (const auto& Event : Timeline) 
    {
        rapidjson::Value EventObject(rapidjson::kObjectType);

        EventObject.AddMember("name", rapidjson::Value(Event.Name, Allocator), Allocator);
        EventObject.AddMember("ph", rapidjson::Value(Event.Phase, Allocator), Allocator);
        EventObject.AddMember("ts", Event.Timestamp, Allocator);
        EventObject.AddMember("pid", Event.ProcessID, Allocator);
        EventObject.AddMember("tid", Event.ThreadID, Allocator);

        TraceEvents.PushBack(EventObject, Allocator);
    }

    Document.AddMember("traceEvents", TraceEvents, Allocator);

    rapidjson::StringBuffer Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
    Document.Accept(Writer);

    std::ofstream File("trace.json");
    File << Buffer.GetString();
    File.close();

    Timeline.clear();
}

void PProfiler::StartEventBlock(const char* Name)
{
    Timeline.emplace_back();

    SEvent& EventData = Timeline.back();
    EventData.Timestamp = static_cast<int64_t>(GetEngine()->Time.GetElapsedTimeAsMicroseconds());
    EventData.Name = Name;
    EventData.Phase = "B";
    EventData.ProcessID = getpid();
    EventData.ThreadID = gettid();
}

void PProfiler::EndEventBlock(const char* Name)
{
    Timeline.emplace_back();

    SEvent& EventData = Timeline.back();
    EventData.Name = Name;
    EventData.Phase = "E";
    EventData.Timestamp = static_cast<int64_t>(GetEngine()->Time.GetElapsedTimeAsMicroseconds());
    EventData.ProcessID = getpid();
    EventData.ThreadID = gettid();
}