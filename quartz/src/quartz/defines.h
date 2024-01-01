#pragma once

#include <stdint.h>

#include "quartz/logging/logger.h"

enum QuartzResult
{
  Quartz_Success = 0,
  Quartz_Failure,
};

namespace Quartz
{
extern uint32_t g_quartzAttemptDepth;
}

#define QTZ_ATTEMPT(fn, ...)                                                                  \
{                                                                                             \
  Quartz::g_quartzAttemptDepth++;                                                             \
  QuartzResult result = fn;                                                                   \
  Quartz::g_quartzAttemptDepth--;                                                             \
  if (result != Quartz_Success)                                                               \
  {                                                                                           \
    QTZ_ERROR("{} : \"{}\"\n\t{}:{}", Quartz::g_quartzAttemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                         \
      __VA_ARGS__;                                                                            \
    }                                                                                         \
    return Quartz_Failure;                                                                    \
  }                                                                                           \
}

#define QTZ_ATTEMPT_FAIL_LOG(...)             \
{                                             \
    QTZ_ERROR(__VA_ARGS__);                   \
    QTZ_ERROR("\t{}:{}", __FILE__, __LINE__); \
}
