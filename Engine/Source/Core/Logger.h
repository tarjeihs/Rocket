#pragma once

#include "spdlog/spdlog.h"

// Ignores all warnings raised inside external header files
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

class PLogger
{
public:
	static void Init();

	static inline std::shared_ptr<spdlog::logger>& GetEngineLogger()
	{
		return GEngineLogger;
	}

private:
	static std::shared_ptr<spdlog::logger> GEngineLogger;
};

#define RK_ENGINE_TRACE(...) ::PLogger::GetEngineLogger()->trace(__VA_ARGS__)
#define RK_ENGINE_VERBOSE(...) ::PLogger::GetEngineLogger()->debug(__VA_ARGS__)
#define RK_ENGINE_INFO(...)  ::PLogger::GetEngineLogger()->info(__VA_ARGS__)
#define RK_ENGINE_WARNING(...)  ::PLogger::GetEngineLogger()->warn(__VA_ARGS__)
#define RK_ENGINE_ERROR(...) ::PLogger::GetEngineLogger()->error(__VA_ARGS__)