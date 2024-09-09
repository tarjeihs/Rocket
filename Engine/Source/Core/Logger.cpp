#include "EnginePCH.h"
#include "Logger.h"

// Ignores all warnings raised inside external header files
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

// Used for spdlog set_level function
#include "spdlog/sinks/stdout_color_sinks.h"

static std::shared_ptr<spdlog::logger> GLogger;

void PLogger::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	GLogger = spdlog::stderr_color_mt("Engine");
	GLogger->set_level(spdlog::level::trace);
}

void PLogger::LogImpl(ELogCategory LogCat, const std::string& Message)
{
	switch (LogCat)
	{
		case ELogCategory::LOG_TRACE: GLogger->trace(Message); break;
		case ELogCategory::LOG_DEBUG: GLogger->debug(Message); break;
		case ELogCategory::LOG_INFO: GLogger->info(Message); break;
		case ELogCategory::LOG_WARNING: GLogger->warn(Message); break;
		case ELogCategory::LOG_ERROR: GLogger->error(Message); break;
		
		default: break; 
	}
}