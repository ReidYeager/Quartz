#pragma once

#include "quartz/defines.h"

#include <opal.h>
#include <peridot.h>

namespace Quartz
{

#define QTZ_ATTEMPT_OPAL(fn, ...)                                                                    \
{                                                                                                    \
  Quartz::g_quartzAttemptDepth++;                                                                    \
  OpalResult result = fn;                                                                            \
  Quartz::g_quartzAttemptDepth--;                                                                    \
  if (result != Opal_Success)                                                                        \
  {                                                                                                  \
    QTZ_ERROR("Opal : {} : \"{}\"\n\t{}:{}", Quartz::g_quartzAttemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                \
      __VA_ARGS__;                                                                                   \
    }                                                                                                \
    return Quartz_Failure;                                                                           \
  }                                                                                                  \
}

struct Vertex
{
  Vec3 position;
  //Vec2 uv;
  //Vec3 normal;
};

} // namespace Quartz