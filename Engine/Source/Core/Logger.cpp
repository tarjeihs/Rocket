#include "EnginePCH.h"
#include "Logger.h"

// Used for spdlog set_level function
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> PLogger::GLogger;

void PLogger::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	GLogger = spdlog::stderr_color_mt("Engine");
	GLogger->set_level(spdlog::level::trace);
}