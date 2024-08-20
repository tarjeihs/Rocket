#pragma once

#include "spdlog/spdlog.h"

// Ignores all warnings raised inside external header files
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

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

	static inline std::shared_ptr<spdlog::logger>& GetLogger()
	{
		return GLogger;
	}

private:
	static std::shared_ptr<spdlog::logger> GLogger;
};

#define RK_LOG_TRACE(...) ::PLogger::GetLogger()->trace(__VA_ARGS__)
#define RK_LOG_VERBOSE(...) ::PLogger::GetLogger()->debug(__VA_ARGS__)
#define RK_LOG_INFO(...)  ::PLogger::GetLogger()->info(__VA_ARGS__)
#define RK_LOG_WARNING(...)  ::PLogger::GetLogger()->warn(__VA_ARGS__)
#define RK_LOG_ERROR(...) ::PLogger::GetLogger()->error(__VA_ARGS__)