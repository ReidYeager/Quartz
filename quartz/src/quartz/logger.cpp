
#include "quartz.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Quartz
{

std::shared_ptr<spdlog::logger> Logger::coreLogger;
std::shared_ptr<spdlog::logger> Logger::appLogger;

void Logger::Init()
{
  spdlog::set_pattern("%^[%T.%e] %2L : %6n :-: %v%$");
  coreLogger = spdlog::stdout_color_mt("Quartz");
  appLogger = spdlog::stdout_color_mt("App");

#ifdef QTZ_CONFIG_DEBUG
  coreLogger->set_level(spdlog::level::trace);
  appLogger->set_level(spdlog::level::trace);
#else
  coreLogger->set_level(spdlog::level::err);
  appLogger->set_level(spdlog::level::err);
#endif

  QTZ_LOG_CORE_INFO("Logging initialized");
}

} // namespace Quartz
