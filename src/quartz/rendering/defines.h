#pragma once

#include "quartz/defines.h"

#include <opal.h>
#define PERIDOT_VULKAN
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
    return Quartz_Failure_Vendor;                                                                    \
  }                                                                                                  \
}

struct Vertex
{
  Vec3 position;
  //Vec2 uv;
  //Vec3 normal;

  bool operator==(const Vertex& other) const
  {
    return Vec3Compare(position, other.position); // && Vec3Compare(normal, other.normal) && Vec2Compare(uv, other.uv);
  }
};

} // namespace Quartz