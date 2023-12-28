#pragma once

#include "quartz/defines.h"

#include <opal.h>

#define QTZ_ATTEMPT_OPAL(fn, ...)                                                                                       \
{                                                                                                                       \
  Quartz::globalFunctionCallAttemptDepth++;                                                                             \
  OpalResult result = fn;                                                                                               \
  Quartz::globalFunctionCallAttemptDepth--;                                                                             \
  if (result != Opal_Success)                                                                                           \
  {                                                                                                                     \
    QTZ_LOG_CORE_ERROR("{} : Opal : \"{}\"\n\t{}:{}", Quartz::globalFunctionCallAttemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                                   \
      __VA_ARGS__;                                                                                                      \
    }                                                                                                                   \
    return Quartz_Failure;                                                                                              \
  }                                                                                                                     \
}