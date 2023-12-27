#pragma once

#include <stdint.h>

#include "quartz/logger.h"

enum QuartzResult
{
  Quartz_Success = 0,
  Quartz_Failure,
};

namespace Quartz
{
extern uint32_t globalFunctionCallAttemptDepth;
}

#define QTZ_ATTEMPT(fn, ...)                                                                                     \
{                                                                                                                \
  Quartz::globalFunctionCallAttemptDepth++;                                                                      \
  QuartzResult result = fn;                                                                                      \
  Quartz::globalFunctionCallAttemptDepth--;                                                                      \
  if (result != Quartz_Success)                                                                                  \
  {                                                                                                              \
    QTZ_LOG_CORE_ERROR("{} : \"{}\"\n\t{}:{}", Quartz::globalFunctionCallAttemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                            \
      __VA_ARGS__;                                                                                               \
    }                                                                                                            \
    return Quartz_Failure;                                                                                       \
  }                                                                                                              \
}
