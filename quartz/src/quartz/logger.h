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

  inline static std::shared_ptr<spdlog::logger> GetCoreLogger() { return coreLogger; }
  inline static std::shared_ptr<spdlog::logger> GetAppLogger() { return appLogger; }

private:
  static std::shared_ptr<spdlog::logger> coreLogger;
  static std::shared_ptr<spdlog::logger> appLogger;
};

} // namespace Quartz

#define QTZ_LOG_CORE_DEBUG(...) Quartz::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define QTZ_LOG_CORE_INFO(...) Quartz::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define QTZ_LOG_CORE_WARNING(...) Quartz::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define QTZ_LOG_CORE_ERROR(...) Quartz::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define QTZ_LOG_CORE_FATAL(...) Quartz::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define QTZ_LOG_DEBUG(...) Quartz::Logger::GetAppLogger()->trace(__VA_ARGS__)
#define QTZ_LOG_INFO(...) Quartz::Logger::GetAppLogger()->info(__VA_ARGS__)
#define QTZ_LOG_WARNING(...) Quartz::Logger::GetAppLogger()->warn(__VA_ARGS__)
#define QTZ_LOG_ERROR(...) Quartz::Logger::GetAppLogger()->error(__VA_ARGS__)
#define QTZ_LOG_FATAL(...) Quartz::Logger::GetAppLogger()->critical(__VA_ARGS__)
