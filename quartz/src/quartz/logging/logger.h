#pragma once

#pragma warning(disable:4996) // Output console spam introduced by spdlog
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <memory>

namespace Quartz
{

class Logger
{
public:
  static void Init();

  static std::shared_ptr<spdlog::logger> coreLogger;
  static std::shared_ptr<spdlog::logger> appLogger;
};

} // namespace Quartz

#define QTZ_DEBUG(...) Quartz::Logger::coreLogger->trace(__VA_ARGS__)
#define QTZ_INFO(...) Quartz::Logger::coreLogger->info(__VA_ARGS__)
#define QTZ_WARNING(...) Quartz::Logger::coreLogger->warn(__VA_ARGS__)
#define QTZ_ERROR(...) Quartz::Logger::coreLogger->error(__VA_ARGS__)
#define QTZ_FATAL(...) Quartz::Logger::coreLogger->critical(__VA_ARGS__)

//#define QTZ_LOG_DEBUG(...) Quartz::Logger::GetAppLogger()->trace(__VA_ARGS__)
//#define QTZ_LOG_INFO(...) Quartz::Logger::GetAppLogger()->info(__VA_ARGS__)
//#define QTZ_LOG_WARNING(...) Quartz::Logger::GetAppLogger()->warn(__VA_ARGS__)
//#define QTZ_LOG_ERROR(...) Quartz::Logger::GetAppLogger()->error(__VA_ARGS__)
//#define QTZ_LOG_FATAL(...) Quartz::Logger::GetAppLogger()->critical(__VA_ARGS__)
