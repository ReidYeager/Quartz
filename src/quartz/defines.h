#pragma once

#include "quartz/logging/logger.h"

#define PERIDOT_VULKAN
#define PERIDOT_NO_NAMESPACE
#include <peridot.h>

#include <stdint.h>

enum QuartzResult
{
  Quartz_Success = 0,
  Quartz_Failure,
  Quartz_Failure_Vendor,
};

struct QuartzInitInfo
{
  struct
  {
    Vec2U extents = { 1280, 720 };
    Vec2I position = { 0, 0 };
    const char* title = "Quartz application";
  } window;
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
    return result;                                                                            \
  }                                                                                           \
}

#define QTZ_ATTEMPT_VOID(fn, ...)                                                             \
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
    return;                                                                                   \
  }                                                                                           \
}

#define QTZ_FAIL_LOG(...)                     \
{                                             \
    QTZ_ERROR(__VA_ARGS__);                   \
    QTZ_ERROR("\t{}:{}", __FILE__, __LINE__); \
}
