#pragma once

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

class PLogger
{
public:
	static void Init();
	
	static void LogImpl(ELogCategory LogCat, const std::string& Message);

    template<typename... TArgs>
    static void Log(ELogCategory LogCat, const std::string& Message, TArgs&&... Args)
    {
        std::string Result = std::vformat(Message, std::make_format_args(Args...));
        PLogger::LogImpl(LogCat, Result);
    }
};

#define RK_LOG_TRACE(...) PLogger::Log(ELogCategory::LOG_TRACE, __VA_ARGS__)
#define RK_LOG_DEBUG(...) PLogger::Log(ELogCategory::LOG_DEBUG, __VA_ARGS__)
#define RK_LOG_INFO(...)  PLogger::Log(ELogCategory::LOG_INFO, __VA_ARGS__)
#define RK_LOG_WARNING(...) PLogger::Log(ELogCategory::LOG_WARNING, __VA_ARGS__)
#define RK_LOG_ERROR(...) PLogger::Log(ELogCategory::LOG_ERROR, __VA_ARGS__)