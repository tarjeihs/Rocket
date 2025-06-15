#pragma once

#define SPDLOG_USE_STD_FORMAT
#include <format>
#include <string>
 
enum ELogCategory
{
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

class FLogger
{
public:
	static void Init();
	
	static void LogImpl(ELogCategory LogCat, const std::string& Message);

    template<typename... TArgs>
    static void Log(ELogCategory LogCat, const std::string& Message, TArgs&&... Args)
    {
        std::string Result = std::vformat(Message, std::make_format_args(Args...));
        FLogger::LogImpl(LogCat, Result);
    }
};

#define RK_LOG_TRACE(...)		FLogger::Log(ELogCategory::LOG_TRACE, __VA_ARGS__)
#define RK_LOG_DEBUG(...)		FLogger::Log(ELogCategory::LOG_DEBUG, __VA_ARGS__)
#define RK_LOG_INFO(...)		FLogger::Log(ELogCategory::LOG_INFO, __VA_ARGS__)
#define RK_LOG_WARNING(...)		FLogger::Log(ELogCategory::LOG_WARNING, __VA_ARGS__)
#define RK_LOG_ERROR(...)		FLogger::Log(ELogCategory::LOG_ERROR, __VA_ARGS__)
